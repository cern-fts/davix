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

#ifndef DAVIX_BACKEND_STANDALONE_REQUEST_HPP
#define DAVIX_BACKEND_STANDALONE_REQUEST_HPP

#include <string>
#include <status/DavixStatus.hpp>

namespace Davix {

class Uri;

//------------------------------------------------------------------------------
// Describe current request state.
//------------------------------------------------------------------------------
enum class RequestState {
  kNotStarted = 0,      // request object has been constructed,
                        // but startRequest has not been called yet

  kStarted,             // startRequest has been called

  kFinished             // request is done, session has been released
};

//------------------------------------------------------------------------------
// Abstract StandaloneRequest class. Represents a simple, no-magic HTTP request.
// No redirects, request signing or any other extra features.
//------------------------------------------------------------------------------
class StandaloneRequest {
public:
  //----------------------------------------------------------------------------
  // Virtual destructor
  //----------------------------------------------------------------------------
  virtual ~StandaloneRequest() {}

  //----------------------------------------------------------------------------
  // Get a specific response header
  //----------------------------------------------------------------------------
  virtual bool getAnswerHeader(const std::string &header_name, std::string &value) const = 0;

  //----------------------------------------------------------------------------
  // Get all response headers
  //----------------------------------------------------------------------------
  virtual size_t getAnswerHeaders(std::vector<std::pair<std::string, std::string > > & vec_headers) const = 0;

  //----------------------------------------------------------------------------
  // Start request - calling this more than once will not have any effect.
  //----------------------------------------------------------------------------
  virtual Status startRequest() = 0;

  //----------------------------------------------------------------------------
  // Finish an already started request.
  //----------------------------------------------------------------------------
  virtual Status endRequest() = 0;

  //----------------------------------------------------------------------------
  // Major read function - read a block of max_size bytes (at max) into buffer.
  //----------------------------------------------------------------------------
  virtual dav_ssize_t readBlock(char* buffer, dav_size_t max_size, Status& st) = 0;

  //----------------------------------------------------------------------------
  // Check request state
  //----------------------------------------------------------------------------
  virtual RequestState getState() const = 0;

  //----------------------------------------------------------------------------
  // Get status code - returns 0 if impossible to determine
  //----------------------------------------------------------------------------
  virtual int getStatusCode() const = 0;

  //----------------------------------------------------------------------------
  // Do not re-use underlying session
  //----------------------------------------------------------------------------
  virtual void doNotReuseSession() = 0;

  //----------------------------------------------------------------------------
  // Has the underlying session been used before?
  //----------------------------------------------------------------------------
  virtual bool isRecycledSession() const = 0;

  //----------------------------------------------------------------------------
  // Obtain redirected location, store into the given Uri
  //----------------------------------------------------------------------------
  virtual Status obtainRedirectedLocation(Uri &out) = 0;

  //----------------------------------------------------------------------------
  // Get session error, if available
  //----------------------------------------------------------------------------
  virtual std::string getSessionError() const = 0;
};

}

#endif