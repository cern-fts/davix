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

#ifndef DAVIX_CURL_SESSION_HPP
#define DAVIX_CURL_SESSION_HPP

#include <memory>
#include <string>

typedef void CURL;
typedef void CURLM;

namespace Davix {

class CurlSessionFactory;
class Uri;
class RequestParams;
class Status;

//------------------------------------------------------------------------------
// CurlHandle, internal use only
//------------------------------------------------------------------------------
struct CurlHandle {
  std::string key;
  CURLM *mhandle;
  CURL *handle;

  void renewHandle();
  CurlHandle(const std::string &k, CURLM *mh, CURL *h);
  CurlHandle() : mhandle(NULL), handle(NULL) {}
  ~CurlHandle();
};

typedef std::shared_ptr<CurlHandle> CurlHandlePtr;

class CurlSession {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  CurlSession(CurlSessionFactory &f, CurlHandlePtr handle, const Uri & uri,
    const RequestParams & p, Status &st);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~CurlSession();

  //----------------------------------------------------------------------------
  // Get handle
  //----------------------------------------------------------------------------
  CurlHandle* getHandle() {
    return _handle.get();
  }

private:
  //----------------------------------------------------------------------------
  // Configure session
  //----------------------------------------------------------------------------
  void configureSession(const RequestParams &params, Status &st);

  CurlSessionFactory &_factory;
  CurlHandlePtr _handle;
};

}

#endif
