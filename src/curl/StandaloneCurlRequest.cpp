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

#include "StandaloneCurlRequest.hpp"
#include "CurlSessionFactory.hpp"
#include "CurlSession.hpp"
#include <curl/curl.h>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

namespace Davix {

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
StandaloneCurlRequest::StandaloneCurlRequest(CurlSessionFactory &sessionFactory, bool reuseSession,
  const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
  const std::vector<HeaderLine> &headers, int reqFlag, ContentProvider *contentProvider,
  Chrono::TimePoint deadline)
: _session_factory(sessionFactory), _reuse_session(reuseSession), _bound_hooks(boundHooks),
  _uri(uri), _verb(verb), _params(params), _headers(headers), _req_flag(reqFlag),
  _content_provider(contentProvider), _deadline(deadline), _state(RequestState::kNotStarted) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
StandaloneCurlRequest::~StandaloneCurlRequest() {}

//------------------------------------------------------------------------------
// Get a specific response header
//------------------------------------------------------------------------------
bool StandaloneCurlRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {

}

//------------------------------------------------------------------------------
// Get all response headers
//------------------------------------------------------------------------------
size_t StandaloneCurlRequest::getAnswerHeaders(std::vector<std::pair<std::string, std::string > > & vec_headers) const {

}

//------------------------------------------------------------------------------
// Start request - calling this multiple times will do nothing.
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::startRequest() {
  if(_state != RequestState::kNotStarted) {
    return Status(); ;
  }

  //----------------------------------------------------------------------------
  // Have we timed out already?
  //----------------------------------------------------------------------------
  Status st = checkTimeout();
  if(!st.ok()) {
    // markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Retrieve a session, create request
  //----------------------------------------------------------------------------
  _session = _session_factory.provideCurlSession(_uri, _params, st);
  if(!st.ok()) {
    // markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Set request verb, target URL
  //----------------------------------------------------------------------------
  CURL* handle = _session->getHandle()->handle;

  curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, _verb.c_str());
  curl_easy_setopt(handle, CURLOPT_URL, _uri.getString());

  //----------------------------------------------------------------------------
  // Set-up headers
  //----------------------------------------------------------------------------
  struct curl_slist *chunk = NULL;

  for(size_t i = 0; i < _headers.size(); i++) {
    chunk = curl_slist_append(chunk, SSTR(_headers[i].first << ": " << _headers[i].second).c_str());
  }

  //----------------------------------------------------------------------------
  // Assign easy handle to multi
  //----------------------------------------------------------------------------
  curl_multi_add_handle(_session->getHandle()->mhandle, handle);



}

//------------------------------------------------------------------------------
// Major read function - read a block of max_size bytes (at max) into buffer.
//------------------------------------------------------------------------------
dav_ssize_t StandaloneCurlRequest::readBlock(char* buffer, dav_size_t max_size, Status& st) {
  if(!_session) {
    st = Status(davix_scope_http_request(), StatusCode::AlreadyRunning, "Request has not been started yet");
    return -1;
  }

}

//------------------------------------------------------------------------------
// Finish an already started request.
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::endRequest() {

}

//------------------------------------------------------------------------------
// Check request state
//------------------------------------------------------------------------------
RequestState StandaloneCurlRequest::getState() const {

}


//------------------------------------------------------------------------------
// Check if timeout has passed
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::checkTimeout() {
  if(_deadline.isValid() && _deadline < Chrono::Clock(Chrono::Clock::Monolitic).now()) {
    std::ostringstream ss;
    ss << "timeout of " << _params.getOperationTimeout()->tv_sec << "s";
    return Status(davix_scope_http_request(), StatusCode::OperationTimeout, ss.str());
  }

  return Status();
}

//------------------------------------------------------------------------------
// Get status code - returns 0 if impossible to determine
//------------------------------------------------------------------------------
int StandaloneCurlRequest::getStatusCode() const {
  CURL* handle = _session->getHandle()->handle;
  long response_code = 0;
  curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
  return response_code;
}

//------------------------------------------------------------------------------
// Do not re-use underlying session
//------------------------------------------------------------------------------
void StandaloneCurlRequest::doNotReuseSession() {

}

//------------------------------------------------------------------------------
// Has the underlying session been used before?
//------------------------------------------------------------------------------
bool StandaloneCurlRequest::isRecycledSession() const {

}

//------------------------------------------------------------------------------
// Obtain redirected location, store into the given Uri
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::obtainRedirectedLocation(Uri &out) {

}

//------------------------------------------------------------------------------
// Get session error, if available
//------------------------------------------------------------------------------
std::string StandaloneCurlRequest::getSessionError() const {

}




}