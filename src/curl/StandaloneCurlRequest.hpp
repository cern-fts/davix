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

#ifndef DAVIX_STANDALONE_CURL_REQUEST_HPP
#define DAVIX_STANDALONE_CURL_REQUEST_HPP

#include "ResponseBuffer.hpp"
#include <davix_internal.hpp>
#include <backend/StandaloneRequest.hpp>
#include <backend/BoundHooks.hpp>
#include <params/davixrequestparams.hpp>

struct curl_slist;

namespace Davix {

class CurlSessionFactory;
class ContentProvider;
class CurlSession;

//------------------------------------------------------------------------------
// Implementation of StandaloneRequest interface based on libcurl.
//------------------------------------------------------------------------------
class StandaloneCurlRequest : public StandaloneRequest {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  StandaloneCurlRequest(CurlSessionFactory &sessionFactory, bool reuseSession,
    const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
    const std::vector<HeaderLine> &headers, int requestFlag, ContentProvider *contentProvider,
    Chrono::TimePoint deadline);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~StandaloneCurlRequest();

  //----------------------------------------------------------------------------
  // No evil constructors, no move semantics.
  //----------------------------------------------------------------------------
  StandaloneCurlRequest(const StandaloneCurlRequest& other) = delete;
  StandaloneCurlRequest(StandaloneCurlRequest&& other) = delete;
  StandaloneCurlRequest& operator=(StandaloneCurlRequest&& other) = delete;
  StandaloneCurlRequest& operator=(const StandaloneCurlRequest& other) = delete;

  //----------------------------------------------------------------------------
  // Get a specific response header
  //----------------------------------------------------------------------------
  virtual bool getAnswerHeader(const std::string &header_name, std::string &value) const;

  //----------------------------------------------------------------------------
  // Get all response headers
  //----------------------------------------------------------------------------
  virtual size_t getAnswerHeaders(std::vector<std::pair<std::string, std::string > > & vec_headers) const;

  //----------------------------------------------------------------------------
  // Start request - calling this more than once will not have any effect.
  //----------------------------------------------------------------------------
  virtual Status startRequest();

  //----------------------------------------------------------------------------
  // Finish an already started request.
  //----------------------------------------------------------------------------
  virtual Status endRequest();

  //----------------------------------------------------------------------------
  // Major read function - read a block of max_size bytes (at max) into buffer.
  //----------------------------------------------------------------------------
  virtual dav_ssize_t readBlock(char* buffer, dav_size_t max_size, Status& st);

  //----------------------------------------------------------------------------
  // Check request state
  //----------------------------------------------------------------------------
  virtual RequestState getState() const;

  //----------------------------------------------------------------------------
  // Get status code - returns 0 if impossible to determine
  //----------------------------------------------------------------------------
  virtual int getStatusCode() const;

  //----------------------------------------------------------------------------
  // Do not re-use underlying session
  //----------------------------------------------------------------------------
  virtual void doNotReuseSession();

  //----------------------------------------------------------------------------
  // Has the underlying session been used before?
  //----------------------------------------------------------------------------
  virtual bool isRecycledSession() const;

  //----------------------------------------------------------------------------
  // Obtain redirected location, store into the given Uri
  //----------------------------------------------------------------------------
  virtual Status obtainRedirectedLocation(Uri &out);

  //----------------------------------------------------------------------------
  // Get session error, if available
  //----------------------------------------------------------------------------
  virtual std::string getSessionError() const;

  //----------------------------------------------------------------------------
  // Feed response header
  //----------------------------------------------------------------------------
  void feedResponseHeader(const std::string &header);

private:
  CurlSessionFactory &_session_factory;
  bool _reuse_session;
  BoundHooks _bound_hooks;
  Uri _uri;
  std::string _verb;
  RequestParams _params;
  std::vector<HeaderLine> _headers;
  int _req_flag;
  ContentProvider *_content_provider;
  Chrono::TimePoint _deadline;

  RequestState _state;

  std::unique_ptr<CurlSession> _session;
  Status sessionError;

  //----------------------------------------------------------------------------
  // Check if timeout has passed
  //----------------------------------------------------------------------------
  Status checkTimeout();

  //----------------------------------------------------------------------------
  // Get remaining number of milliseconds until deadline
  //----------------------------------------------------------------------------
  uint64_t getRemainingMs() const;

  //----------------------------------------------------------------------------
  // Check internal mhandle errors
  //----------------------------------------------------------------------------
  Status checkErrors();

  //----------------------------------------------------------------------------
  // Linked list for storing request headers
  //----------------------------------------------------------------------------
  struct curl_slist *_chunklist;

  //----------------------------------------------------------------------------
  // Response variables
  //----------------------------------------------------------------------------
  std::vector<std::pair<std::string, std::string > > _response_headers;
  bool _received_headers;

  ResponseBuffer _response_buffer;

  //----------------------------------------------------------------------------
  // Block until all response headers have been received
  //----------------------------------------------------------------------------
  Status readResponseHeaders();

  //----------------------------------------------------------------------------
  // Perform a single blocking round of network I/O
  //----------------------------------------------------------------------------
  Status performBlockingRound(int &still_running);

};

}

#endif

