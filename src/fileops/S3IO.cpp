/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2017
 * Author: Georgios Bitzes <georgios.bitzes@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include "S3IO.hpp"
#include <utils/davix_logger_internal.hpp>
#include <xml/S3MultiPartInitiationParser.hpp>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

namespace Davix{

static bool is_s3_operation(IOChainContext & context){
  if(context._reqparams->getProtocol() == RequestProtocol::AwsS3) {
    return true;
  }

  return false;
}

static bool should_use_s3_multipart(IOChainContext & context, dav_size_t size) {
  return is_s3_operation(context) && size > (1024 * 1024 * 512); // 512 MB
}

S3IO::S3IO() {

}

S3IO::~S3IO() {

}

std::string S3IO::initiateMultipart(IOChainContext & iocontext) {
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Initiating multi-part upload for {}", iocontext._uri);

  DavixError * tmp_err=NULL;
  Uri url(iocontext._uri);
  url.addQueryParam("uploads", "");

  PostRequest req(iocontext._context, url, &tmp_err);
  checkDavixError(&tmp_err);

  req.setParameters(iocontext._reqparams);
  req.setRequestBody("");
  req.executeRequest(&tmp_err);
  if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
    httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
      "write error: ", &tmp_err);
  }
  checkDavixError(&tmp_err);

  std::string response = req.getAnswerContent();
  S3MultiPartInitiationParser parser;
  if(parser.parseChunk(response) != 0) {
    DavixError::setupError(&tmp_err, "S3::MultiPart", StatusCode::InvalidServerResponse, "Unable to parse server response for multi-part initiation");
  }
  checkDavixError(&tmp_err);


  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Obtained multi-part upload id {} for {}", parser.getUploadId(), iocontext._uri);
  return parser.getUploadId();
}

std::string S3IO::writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const std::string &uploadId, int partNumber) {
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "writing chunk #{}", partNumber);

  DavixError * tmp_err=NULL;
  Uri url(iocontext._uri);
  url.addQueryParam("uploadId", uploadId);
  url.addQueryParam("partNumber", SSTR(partNumber));

  PutRequest req(iocontext._context, url, &tmp_err);
  checkDavixError(&tmp_err);

  req.setParameters(iocontext._reqparams);
  req.setRequestBody(buff, size);
  req.executeRequest(&tmp_err);
  if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
      httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                           "write error: ", &tmp_err);
  }
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "write result size {}", size);
  checkDavixError(&tmp_err);

  std::string etag;
  if(!req.getAnswerHeader("Etag", etag)) {
    DavixError::setupError(&tmp_err, "S3::MultiPart", StatusCode::InvalidServerResponse, "Unable to retrieve chunk Etag, necessary when committing chunks");
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "chunk #{} written successfully, etag: {}", partNumber, etag);
  return etag;
}

void S3IO::commitChunks(IOChainContext & iocontext,  const std::string &uploadId, const std::vector<std::string> &etags) {
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "committing {} chunks", etags.size());

  std::ostringstream payload;
  payload << "<CompleteMultipartUpload>";
  for(size_t i = 1; i <= etags.size(); i++) {
    payload << "<Part>";
    payload << "<PartNumber>" << i << "</PartNumber>";
    payload << "<ETag>" << etags[i-1] << "</ETag>";
    payload << "</Part>";
  }
  payload << "</CompleteMultipartUpload>";

  DavixError * tmp_err=NULL;
  Uri url(iocontext._uri);
  url.addQueryParam("uploadId", uploadId);

  PostRequest req(iocontext._context, url, &tmp_err);
  req.setParameters(iocontext._reqparams);
  req.setRequestBody(payload.str());
  req.executeRequest(&tmp_err);

  if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
      httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                           "write error: ", &tmp_err);
  }
  checkDavixError(&tmp_err);
}

static dav_ssize_t readFunction(int fd, void* buffer, dav_size_t size) {
  return ::read(fd, buffer, size);
}

dav_ssize_t S3IO::writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size) {
  if(!should_use_s3_multipart(iocontext, size)) {
    CHAIN_FORWARD(writeFromFd(iocontext, fd, size));
  }

  using std::placeholders::_1;
  using std::placeholders::_2;

  DataProviderFun providerFunc = std::bind(readFunction, fd, _1, _2);
  return this->writeFromCb(iocontext, providerFunc, size);
}

dav_ssize_t S3IO::writeFromCb(IOChainContext & iocontext, const DataProviderFun & func, dav_size_t size) {
  if(!should_use_s3_multipart(iocontext, size)) {
    CHAIN_FORWARD(writeFromCb(iocontext, func, size));
  }

  std::string uploadId = initiateMultipart(iocontext);

  size_t remaining = size;
  const dav_size_t MAX_CHUNK_SIZE = 1024 * 1024 * 256; // 256 MB

  std::vector<char> buffer;
  buffer.resize(std::max(MAX_CHUNK_SIZE, size) + 10);

  std::vector<std::string> etags;

  size_t partNumber = 0;
  while(remaining > 0) {
    size_t toRead = std::min(size, MAX_CHUNK_SIZE);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "S3IO write: toRead from cb {}", toRead);

    size_t bytesRead = func(buffer.data(), toRead);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "S3IO write: bytesRead from cb {}", bytesRead);
    if(bytesRead == 0) break; // EOF?

    partNumber++;
    etags.emplace_back(writeChunk(iocontext, buffer.data(), bytesRead, uploadId, partNumber));
   }

   commitChunks(iocontext, uploadId, etags);
}


}