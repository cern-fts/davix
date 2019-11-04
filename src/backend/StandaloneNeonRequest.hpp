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

#ifndef DAVIX_BACKEND_STANDALONE_NEON_REQUEST_HPP
#define DAVIX_BACKEND_STANDALONE_NEON_REQUEST_HPP

#include <backend/StandaloneRequest.hpp>
#include "BoundHooks.hpp"
#include <davix_internal.hpp>
#include <utils/davix_uri.hpp>
#include <ne_request.h>
#include <params/davixrequestparams.hpp>
#include <status/DavixStatus.hpp>
#include <memory>

namespace Davix {

class NEONSessionFactory;
class Uri;
class RequestParams;
class NeonSessionWrapper;

//------------------------------------------------------------------------------
// Represent a single, standalone HTTP request with libneon implementation.
//
// No magic here. No automatic redirections, no cached redirections, no
// compatibility hacks, no automatic retries, no automatic URL signing for S3
// or Azure -- NOTHING. Just a single HTTP request / response.
//
// The magic is to be added by higher-level classes building upon this one.
//------------------------------------------------------------------------------
class StandaloneNeonRequest : public StandaloneRequest {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //
  // sessionFactory: Required to obtain an HTTP session.
  //
  // reuseSession: Whether to re-use a previously created session, if available,
  //               or create a brand-new one.
  //
  // boundHooks: Sometimes the user will set hooks which activate at certain
  //             points during request execution.
  //
  // uri: Target URI to hit.
  //
  // param: Request parameters, authentication, etc.
  //----------------------------------------------------------------------------
  StandaloneNeonRequest(NEONSessionFactory &sessionFactory, bool reuseSession,
    const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
    const std::vector<HeaderLine> &headers, int requestFlag, ContentProvider *contentProvider,
    Chrono::TimePoint deadline);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~StandaloneNeonRequest();

  //----------------------------------------------------------------------------
  // No evil constructors, no move semantics.
  //----------------------------------------------------------------------------
  StandaloneNeonRequest(const StandaloneNeonRequest& other) = delete;
  StandaloneNeonRequest(StandaloneNeonRequest&& other) = delete;
  StandaloneNeonRequest& operator=(StandaloneNeonRequest&& other) = delete;
  StandaloneNeonRequest& operator=(const StandaloneNeonRequest& other) = delete;

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

private:
  NEONSessionFactory &_session_factory;
  bool _reuse_session;
  BoundHooks _bound_hooks;
  Uri _uri;
  std::string _verb;
  RequestParams _params;
  RequestState _state;
  std::vector<HeaderLine> _headers;
  int _req_flag;
  ContentProvider *_content_provider;
  Chrono::TimePoint _deadline;

  friend class NeonSessionWrapper;
  std::unique_ptr<NeonSessionWrapper> _session;
  ne_request* _neon_req;
  dav_ssize_t _total_read_size;
  dav_ssize_t _last_read;

  //----------------------------------------------------------------------------
  // Check if timeout has passed
  //----------------------------------------------------------------------------
  Status checkTimeout();

  //----------------------------------------------------------------------------
  // Mark request as completed, release any resources
  //----------------------------------------------------------------------------
  void markCompleted();

  //----------------------------------------------------------------------------
  // Create davix error object based on errors in the current session,
  // or request
  //----------------------------------------------------------------------------
  Status createError(int ne_status);

};

}

#endif
