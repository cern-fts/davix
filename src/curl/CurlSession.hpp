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

typedef void CURL;
typedef void CURLM;

namespace Davix {

class CurlSessionFactory;
class Uri;
class RequestParams;
class Status;

class CurlSession {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  CurlSession(CurlSessionFactory &f, CURLM *mhandle, CURL *handle, const Uri & uri,
    const RequestParams & p, Status &st);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~CurlSession();

private:
  //----------------------------------------------------------------------------
  // Configure session
  //----------------------------------------------------------------------------
  void configureSession(const RequestParams &params, Status &st);

  CurlSessionFactory &_factory;
  CURLM *_mhandle;
  CURL *_handle;

};

}

#endif
