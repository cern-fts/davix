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
#include <core/ContentProvider.hpp>
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
  bool is_s3 = is_s3_operation(context);

  if(!is_s3) return false;

  if(context._uri.fragmentParamExists("forceMultiPart")) {

    return true;
  }

  return size > (1024 * 1024 * 512); // 512 MB
}

S3IO::S3IO() {

}

S3IO::~S3IO() {

}

std::string S3IO::initiateMultipart(IOChainContext & iocontext) {
  Uri url(iocontext._uri);
  url.addQueryParam("uploads", "");

  return initiateMultipart(iocontext, url);
}

std::string S3IO::initiateMultipart(IOChainContext & iocontext, const Uri &url) {
  DavixError * tmp_err=NULL;

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
  Uri url(iocontext._uri);
  url.addQueryParam("uploadId", uploadId);
  url.addQueryParam("partNumber", SSTR(partNumber));

  return writeChunk(iocontext, buff, size, url, partNumber);
}

std::string S3IO::writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const Uri &url, int partNumber) {
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "writing chunk #{} with size {}", partNumber, size);

  DavixError * tmp_err=NULL;
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
  Uri url(iocontext._uri);
  url.addQueryParam("uploadId", uploadId);

  return commitChunks(iocontext, url, etags);
}

void S3IO::commitChunks(IOChainContext & iocontext,  const Uri &url, const std::vector<std::string> &etags) {
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

static dav_size_t fillBufferWithProviderData(std::vector<char> &buffer, const dav_size_t maxChunkSize, ContentProvider &provider) {
    dav_size_t written = 0u;
    dav_size_t remaining = maxChunkSize;

    while(true) {
      dav_ssize_t bytesRead = provider.pullBytes(buffer.data(), remaining);
      if(bytesRead < 0) {
        throw DavixException(davix_scope_io_buff(), StatusCode::InvalidFileHandle, fmt::format("Error when reading from callback: {}", bytesRead));
      }

      remaining -= bytesRead;
      written += bytesRead;

      if(bytesRead == 0) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Reached data provider EOF, received 0 bytes, even though asked for {}", remaining);
        break; // EOF
      }

      if(remaining == 0) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Data provider buffer has been filled");
        break; // buffer is full
      }
    }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Retrieved {} bytes from data provider", written);
    return written;
}

// write from content provider
dav_ssize_t S3IO::writeFromProvider(IOChainContext & iocontext, ContentProvider &provider) {
  if(!should_use_s3_multipart(iocontext, provider.getSize())) {
    CHAIN_FORWARD(writeFromProvider(iocontext, provider));
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Initiating multi-part upload towards {} to upload file with size {}", iocontext._uri, provider.getSize());
  std::string uploadId = initiateMultipart(iocontext);

  size_t remaining = provider.getSize();
  const dav_size_t MAX_CHUNK_SIZE = 1024 * 1024 * 256; // 256 MB

  std::vector<char> buffer;
  buffer.resize(std::min(MAX_CHUNK_SIZE, (dav_size_t) provider.getSize()) + 10);

  std::vector<std::string> etags;

  size_t partNumber = 0;
  while(remaining > 0) {
    size_t toRead = std::min( (dav_size_t) provider.getSize(), MAX_CHUNK_SIZE);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "S3IO write: toRead from cb {}", toRead);

    dav_size_t bytesRead = fillBufferWithProviderData(buffer, MAX_CHUNK_SIZE, provider);
    if(bytesRead == 0) break; // EOF

    partNumber++;
    etags.emplace_back(writeChunk(iocontext, buffer.data(), bytesRead, uploadId, partNumber));
  }

  commitChunks(iocontext, uploadId, etags);
  return provider.getSize();
}

DynafedUris S3IO::retrieveDynafedUris(IOChainContext & iocontext, const std::string &uploadId, const std::string &pluginId, size_t nchunks) {
  DynafedUris retval;

  DavixError *tmp_err = NULL;
  PutRequest req(iocontext._context, iocontext._uri, &tmp_err);
  checkDavixError(&tmp_err);

  req.setParameters(iocontext._reqparams);
  req.addHeaderField("x-s3-uploadid", uploadId);
  req.addHeaderField("x-ugrpluginid", pluginId);
  req.addHeaderField("x-s3-upload-nchunks", SSTR(nchunks));
  req.executeRequest(&tmp_err);

  if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
      httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                           "write error: ", &tmp_err);
  }
  checkDavixError(&tmp_err);

  // We have a response, parse it into lines
  retval.chunks = StrUtil::tokenSplit(req.getAnswerContent(), "\n");
  if(!retval.chunks.empty()) {
    retval.post = retval.chunks.back();
    retval.chunks.pop_back();
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "retrieveDynafedUris: Obtained list with {} chunk URIs in total", retval.chunks.size());
  return retval;
}

void S3IO::performUgrS3MultiPart(IOChainContext & iocontext, const std::string &posturl, const std::string &pluginId, ContentProvider &provider, DavixError **err) {
    try {
        Uri uri(posturl);
        std::string uploadId = initiateMultipart(iocontext, posturl);

        const dav_size_t MAX_CHUNK_SIZE = 1024 * 1024 * 256; // 256 MB
        std::vector<char> buffer;
        buffer.resize(std::min(MAX_CHUNK_SIZE, (dav_size_t) provider.getSize()) + 10);

        size_t nchunks = (provider.getSize() / MAX_CHUNK_SIZE) + 2;
        DynafedUris uris = retrieveDynafedUris(iocontext, uploadId, pluginId, nchunks);

        if(uris.chunks.size() != nchunks) {
          DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CHAIN, "Dynafed returned different number of URIs than expected: {} vs {]", "} retrieveDynafedUris: Obtained list with {} chunk URIs in total", uris.chunks.size(), nchunks);
          throw DavixException("S3::MultiPart", StatusCode::InvalidServerResponse, "Dynafed returned different number of URIs than expected");
        }

        std::vector<std::string> etags;
        size_t partNumber = 1;
        uint64_t remaining = provider.getSize();

        while(remaining > 0) {
          dav_size_t bytesRetrieved = fillBufferWithProviderData(buffer, MAX_CHUNK_SIZE, provider);
          if(bytesRetrieved == 0) {
            break; // EOF
          }

          etags.emplace_back(writeChunk(iocontext, buffer.data(), bytesRetrieved, Uri(uris.chunks[partNumber-1]), partNumber));
          partNumber++;
          remaining -= bytesRetrieved;
        }

        commitChunks(iocontext, Uri(uris.post), etags);
    }
    CATCH_DAVIX(err);
}


}
