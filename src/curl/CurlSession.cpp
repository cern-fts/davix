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

#include "CurlSession.hpp"
#include <curl/curl.h>
#include <params/davixrequestparams.hpp>
#include <mutex>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

namespace Davix {

static std::once_flag curl_once;

void init_curl() {
  curl_global_init(CURL_GLOBAL_ALL);
}

//------------------------------------------------------------------------------
// CurlHandle: Destructor
//------------------------------------------------------------------------------
CurlHandle::~CurlHandle() {
  if(handle) {
    curl_easy_cleanup(handle);
  }

  if(mhandle) {
    curl_multi_cleanup(mhandle);
  }
}

//------------------------------------------------------------------------------
// CurlHandle: Constructor
//------------------------------------------------------------------------------
CurlHandle::CurlHandle(const std::string &k, CURLM *mh, CURL *h) : key(k), mhandle(mh), handle(h) {
  curl_multi_add_handle(mhandle, handle);
}

//------------------------------------------------------------------------------
// Renew curl handle
//------------------------------------------------------------------------------
void CurlHandle::renewHandle() {
  if(handle) {
    curl_easy_cleanup(handle);
  }

  handle = curl_easy_init();
  curl_multi_add_handle(mhandle, handle);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
CurlSession::CurlSession(CurlSessionFactory &f, CurlHandlePtr h, const Uri & uri, const RequestParams & p, Status &st)
: _factory(f), _handle(h) {

  configureSession(p, st);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
CurlSession::~CurlSession() {}

//------------------------------------------------------------------------------
// Configure session
//------------------------------------------------------------------------------
void CurlSession::configureSession(const RequestParams &params, Status &st) {
  if(!params.getSSLCACheck()) {
    curl_easy_setopt(_handle->handle, CURLOPT_SSL_VERIFYPEER, 0L);
  }
}



}