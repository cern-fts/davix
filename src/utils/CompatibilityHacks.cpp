/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
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

#include "CompatibilityHacks.hpp"
#include <fileops/AzureIO.hpp>
#include <fileops/S3IO.hpp>
#include <utils/davix_logger_internal.hpp>

namespace Davix {

static dav_ssize_t iocontext_content_provider(HttpBodyProvider provider, void* udata, void* buffer, dav_size_t size) {
    return provider(udata, (char*) buffer, size);
}

static dav_ssize_t readFunction(int fd, void* buffer, dav_size_t size) {
    dav_ssize_t ret = ::read(fd, buffer, size);
    if(ret < 0) {
        int myerr = errno;
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Error in readFunction when attempting to read from fd {}: Return code {}, errno: {}", fd, ret, myerr);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Azure has a rather restrictive limit of 256 MB uploaded per each HTTP
// request.
//
// In case more than 256MB needs to be uploaded, we follow Azure's chunked
// upload mechanism and commit at the end.
//
// All is good if davix _knows_ we're writing to Azure. What happens if we're
// just redirected to a pre-signed Azure endpoint?
//
// We need to guess if we're talking to Azure after a redirect, and engage
// the chunking mechanim as before.
//
// Function returns true if Azure mechanism was engaged, false otherwise.
// (False means "This is not an Azure URL")
//------------------------------------------------------------------------------
bool CompatibilityHacks::azureChunkedUpload(const std::string &requestType, const Uri& uri, Context &context, const RequestParams &params, int fdContent, dav_size_t contentLen, DavixError **err, ContentProviderContext &contentProvider) {
  if(!shouldEngageAzureChunkedUpload(requestType, uri)) {
    return false;
  }

  IOChainContext iocontext(context, uri, &params);

  using std::placeholders::_1;
  using std::placeholders::_2;

  try {
    AzureIO azureio;

    if(fdContent > 0) {
      azureio.writeFromFd(iocontext, fdContent, contentLen);
    }
    else if(contentProvider.callback) {
      DataProviderFun provider = std::bind(iocontext_content_provider, contentProvider.callback, contentProvider.udata, _1, _2);
      azureio.writeFromCb(iocontext, provider, contentLen);
    }
    else {
      throw DavixException("Azure IO", StatusCode::InvalidArgument, "unable to use Azure PUT by providing a direct string");
    }
  }
  catch(DavixException &e) {
    e.toDavixError(err);
  }

  return true;
}

//------------------------------------------------------------------------------
// Heuristic to check if URL + requestType should engage the Azure
// chunk-upload mechanism.
//------------------------------------------------------------------------------
bool CompatibilityHacks::shouldEngageAzureChunkedUpload(const std::string &requestType, const Uri& uri) {
  if(requestType != "PUT" && requestType != "put") {
    return false;
  }

  if(!uri.queryParamExists("sig") || !uri.queryParamExists("sr") || !uri.queryParamExists("sp")) {
    return false;
  }

  if(uri.fragmentParamExists("azuremechanism")) {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// Dynafed-assisted multi-chunk S3 upload.
// Returns if dynafed mechanism was engaged.
//------------------------------------------------------------------------------
bool CompatibilityHacks::dynafedAssistedS3Upload(const BackendRequest& originatingRequest, const Uri& uri, Context& context, const RequestParams &params, int fdContent, dav_size_t contentLen, DavixError** err, ContentProviderContext &contentProvider) {
  std::string ugrs3post;
  std::string ugrpluginid;

  const uint64_t s3SizeThreshold = (1024ull * 1024ull * 1024ull * 5ull);

  if(!originatingRequest.getAnswerHeader("x-ugrs3posturl", ugrs3post)) {
    return false;
  }

  if(!originatingRequest.getAnswerHeader("x-ugrpluginid", ugrpluginid)) {
    return false;
  }

  if(ugrs3post.empty()) {
    return false;
  }

  // Don't engage if size to upload is less than the specified threshold.
  // Override with 'forceMultiPart' fragment param.
  if(contentLen < s3SizeThreshold && !uri.fragmentParamExists("forceMultiPart")) {
    return false;
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Engaging dynafed-assisted multi-part upload to S3, posturl: {}, pluginid: {}", ugrs3post, ugrpluginid);
  IOChainContext iocontext(context, uri, &params);

  using std::placeholders::_1;
  using std::placeholders::_2;

  S3IO s3io;
  DataProviderFun provider;


  if(fdContent > 0) {
    provider = std::bind(readFunction, fdContent, _1, _2);
  }
  else if(contentProvider.callback) {
   provider = std::bind(iocontext_content_provider, contentProvider.callback, contentProvider.udata, _1, _2);
  }

  s3io.performUgrS3MultiPart(iocontext, ugrs3post, ugrpluginid, provider, contentLen, err);
  return true;
}

}