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

#include "AzureIO.hpp"
#include <utils/davix_logger_internal.hpp>
#include <core/ContentProvider.hpp>

#include <iomanip>
#include <uuid/uuid.h>
#include <algorithm>
#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

namespace Davix{

AzureIO::AzureIO() : HttpIOChain() { }

AzureIO::~AzureIO() {

}

static bool is_azure_operation(IOChainContext & context){
  if(context._reqparams->getProtocol() == RequestProtocol::Azure) {
    return true;
  }

  if(context._uri.queryParamExists("sig") &&
     context._uri.queryParamExists("sr")  &&
     context._uri.queryParamExists("sp")) {

    return true;
  }

  return false;
}

static std::string stringifyBlockID(const std::string &prefix, size_t blockid) {
  std::string strblockid = SSTR(prefix << "+" << std::setfill('0') << std::setw(10) << blockid); // TODO ensure fixed size
  return Base64::base64_encode( (unsigned char*) strblockid.c_str(), strblockid.size());
}

void AzureIO::writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const std::string &blockid) {
  DavixError * tmp_err=NULL;
  Uri url(iocontext._uri);
  url.addQueryParam("comp", "block");
  url.addQueryParam("blockid", blockid);
  url.addFragmentParam("azuremechanism", "true");

  PutRequest req(iocontext._context, url, &tmp_err);
  if(!tmp_err){
    RequestParams params(iocontext._reqparams);
    params.addHeader("x-ms-blob-type", "BlockBlob");
    req.setParameters(params);
    req.setRequestBody(buff, size);
    req.executeRequest(&tmp_err);
    if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
        httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                             "write error: ", &tmp_err);
    }
  }
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "write result size {}", size);
  checkDavixError(&tmp_err);
}

void AzureIO::commitChunks(IOChainContext & iocontext, const std::vector<std::string> &blocklist) {
  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Azure write: committing {} blocks", blocklist.size());

  DavixError * tmp_err=NULL;
  Uri url(iocontext._uri);
  url.addQueryParam("comp", "blocklist");
  url.addFragmentParam("azuremechanism", "true");

  std::stringstream body;
  body << "<?xml version=\"1.0\" encoding=\"utf-8\"?><BlockList>";
  for(size_t i = 0; i < blocklist.size(); i++) {
    body << "<Latest>" << blocklist[i] << "</Latest>";
  }
  body << "</BlockList>";

  PutRequest req(iocontext._context, url, &tmp_err);
  if(!tmp_err){
    RequestParams params(iocontext._reqparams);
    req.setParameters(params);
    req.setRequestBody(body.str());
    req.executeRequest(&tmp_err);
    if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
        httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                             "write error: ", &tmp_err);
    }
  }
  checkDavixError(&tmp_err);
}

static std::string uuid_to_string(uuid_t uuid) {
  std::stringstream ss;
  for(size_t i = 0; i < 16; i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int) uuid[i];
  }
  return ss.str();
}

static std::string get_uuid() {
  uuid_t uuid;
  uuid_generate_random(uuid);
  return uuid_to_string(uuid);
}

// write from content provider
dav_ssize_t AzureIO::writeFromProvider(IOChainContext & iocontext, ContentProvider &provider) {
  if(!is_azure_operation(iocontext)) {
    CHAIN_FORWARD(writeFromProvider(iocontext, provider));
  }

  std::vector<std::string> blockIDs;

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Azure write: size {}, splitting into blocks", provider.getSize());
  std::vector<char> buffer;

  const dav_size_t MAX_CHUNK_SIZE = 1024 * 1024 * 100; // 100 MB
  buffer.resize(std::min(MAX_CHUNK_SIZE, (dav_size_t) provider.getSize()) + 10);

  // generate UUID to use as blockid prefix
  std::string prefix = get_uuid();

  size_t blockid = 0;
  size_t remaining = provider.getSize();
  while(remaining > 0) {
    size_t toRead = std::min( (dav_size_t) provider.getSize(), MAX_CHUNK_SIZE);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Azure write: toRead from cb {}", toRead);

    size_t bytesRead = provider.pullBytes(buffer.data(), toRead);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Azure write: bytesRead from cb {}", bytesRead);
    if(bytesRead == 0) break; // EOF?

    blockIDs.push_back(stringifyBlockID(prefix, blockid));
    writeChunk(iocontext, buffer.data(), bytesRead, blockIDs.back());
    blockid++;

    remaining -= bytesRead;
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Azure write: remaining bytes {}", remaining);
  }

  // Now let's commit the blobs
  commitChunks(iocontext, blockIDs);
  return provider.getSize();

}


}
