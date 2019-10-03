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

#ifndef DAVIX_COMPATIBILITY_HACKS_HPP
#define DAVIX_COMPATIBILITY_HACKS_HPP

#include <utils/davix_uri.hpp>
#include <params/davixrequestparams.hpp>
#include <string>
#include "../backend/BackendRequest.hpp"


namespace Davix {

class BackendRequest;

//------------------------------------------------------------------------------
// A selection of compatibility hacks and procedures. Proceed with care.
//------------------------------------------------------------------------------
class CompatibilityHacks {
public:
  //----------------------------------------------------------------------------
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
  //----------------------------------------------------------------------------
  static bool azureChunkedUpload(const std::string &requestType, const Uri& uri, Context &context, const RequestParams &params, ContentProvider &provider, DavixError** err);

  //----------------------------------------------------------------------------
  // Heuristic to check if URL + requestType should engage the Azure
  // chunk-upload mechanism.
  //----------------------------------------------------------------------------
  static bool shouldEngageAzureChunkedUpload(const std::string &requestType, const Uri& uri);

  //----------------------------------------------------------------------------
  // Dynafed-assisted multi-chunk S3 upload.
  // Returns if dynafed mechanism was engaged.
  //----------------------------------------------------------------------------
  static bool dynafedAssistedS3Upload(const BackendRequest &originatingRequest, const Uri& uri, Context &context, const RequestParams &params, ContentProvider &provider, DavixError** err);




};


}

#endif