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

#include "CurlSessionFactory.hpp"
#include "CurlSession.hpp"
#include <backend/SessionFactory.hpp>
#include <curl/curl.h>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

namespace Davix {

//------------------------------------------------------------------------------
// Check if session caching is disabled from environment variables
//------------------------------------------------------------------------------
static bool isSessionCachingDisabled(){
  return ( getenv("DAVIX_DISABLE_SESSION_CACHING") != NULL);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
CurlSessionFactory::CurlSessionFactory() : _session_caching(!isSessionCachingDisabled()) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
CurlSessionFactory::~CurlSessionFactory() {}

//------------------------------------------------------------------------------
// Create a CurlSession tied to this class.
//------------------------------------------------------------------------------
std::unique_ptr<CurlSession> CurlSessionFactory::provideCurlSession(const Uri &uri, const RequestParams &params, Status &st) {
  CurlHandlePtr handle = getCachedHandle(uri, params);

  if(!handle) {
    handle = makeNewHandle(uri, params);
  }

  std::unique_ptr<CurlSession> out(new CurlSession(*this, handle, uri, params, st));
  if(st.ok()) {
    return out;
  }

  out.reset();
  handle.reset();

  handle = makeNewHandle(uri, params);
  out.reset(new CurlSession(*this, handle, uri, params, st));
  return out;
}

//------------------------------------------------------------------------------
// Set caching on or off
//------------------------------------------------------------------------------
void CurlSessionFactory::setSessionCaching(bool caching) {
  std::lock_guard<std::mutex> lock(_session_caching_mtx);
  _session_caching = caching && !isSessionCachingDisabled();
}

//------------------------------------------------------------------------------
// Get caching status
//------------------------------------------------------------------------------
bool CurlSessionFactory::getSessionCaching() const {
  std::lock_guard<std::mutex> lock(_session_caching_mtx);
  return _session_caching;
}

//------------------------------------------------------------------------------
// Retrieve cached handle, if possible
//------------------------------------------------------------------------------
CurlHandlePtr CurlSessionFactory::getCachedHandle(const Uri &uri, const RequestParams &params) {
  CurlHandlePtr out;
  std::string sessionKey = SessionFactory::makeSessionKey(uri);

  if(_session_pool.retrieve(sessionKey, out)) {
    out->renewHandle();
  }

  return out;
}

//------------------------------------------------------------------------------
// Provide brand-new handle
//------------------------------------------------------------------------------
CurlHandlePtr CurlSessionFactory::makeNewHandle(const Uri &uri, const RequestParams &params) {
  std::string sessionKey = SessionFactory::makeSessionKey(uri);

  CURL *handle = curl_easy_init();
  CURLM *mhandle = curl_multi_init();

  return CurlHandlePtr(new CurlHandle(sessionKey, mhandle, handle));
}

}
