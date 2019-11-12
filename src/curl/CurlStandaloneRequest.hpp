/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#ifndef DAVIX_CURL_REQUEST_HPP
#define DAVIX_CURL_REQUEST_HPP

#include <davix_internal.hpp>
#include <backend/StandaloneRequest.hpp>
#include <backend/BoundHooks.hpp>
#include <params/davixrequestparams.hpp>

namespace Davix {

class CurlSessionFactory;
class ContentProvider;

//------------------------------------------------------------------------------
// Implementation of StandaloneRequest interface based on libcurl.
//------------------------------------------------------------------------------
class CurlStandaloneRequest : public StandaloneRequest {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  CurlStandaloneRequest(CurlSessionFactory &sessionFactory, bool reuseSession,
    const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
    const std::vector<HeaderLine> &headers, int requestFlag, ContentProvider *contentProvider,
    Chrono::TimePoint deadline);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~CurlStandaloneRequest();

  //----------------------------------------------------------------------------
  // No evil constructors, no move semantics.
  //----------------------------------------------------------------------------
  CurlStandaloneRequest(const CurlStandaloneRequest& other) = delete;
  CurlStandaloneRequest(CurlStandaloneRequest&& other) = delete;
  CurlStandaloneRequest& operator=(CurlStandaloneRequest&& other) = delete;
  CurlStandaloneRequest& operator=(const CurlStandaloneRequest& other) = delete;


};

}

#endif

