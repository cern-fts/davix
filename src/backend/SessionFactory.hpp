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

#ifndef DAVIX_SESSION_FACTORY_HPP
#define DAVIX_SESSION_FACTORY_HPP

#include <davix_internal.hpp>

#include <mutex>
#include <memory>

namespace Davix {

class NEONSessionFactory;
class CurlSessionFactory;
class Uri;

//------------------------------------------------------------------------------
// Store session factories of both neon and curl implementations.
//------------------------------------------------------------------------------
class SessionFactory {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  SessionFactory();

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~SessionFactory();

  //----------------------------------------------------------------------------
  // Get neon session factory
  //----------------------------------------------------------------------------
  NEONSessionFactory& getNeon();

  //----------------------------------------------------------------------------
  // Get curl session factory
  //----------------------------------------------------------------------------
  CurlSessionFactory& getCurl();

  //----------------------------------------------------------------------------
  // Set caching on or off
  //----------------------------------------------------------------------------
  void setSessionCaching(bool caching);

  //----------------------------------------------------------------------------
  // Get caching status
  //----------------------------------------------------------------------------
  bool getSessionCaching() const;

  //----------------------------------------------------------------------------
  // "httpize" protocol
  //----------------------------------------------------------------------------
  static std::string httpizeProtocol(const std::string &protocol);

  //----------------------------------------------------------------------------
  // Create session key based on Uri
  //----------------------------------------------------------------------------
  static std::string makeSessionKey(const Uri &uri);

protected:
  std::unique_ptr<NEONSessionFactory> _neon_factory;
  std::unique_ptr<CurlSessionFactory> _curl_factory;

};

}

#endif

