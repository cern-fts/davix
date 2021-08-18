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

#include "SessionFactory.hpp"
#include <stdlib.h>
#include <neon/neonsessionfactory.hpp>
#include <curl/CurlSessionFactory.hpp>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

namespace Davix {

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
SessionFactory::SessionFactory() {
  _neon_factory.reset(new NEONSessionFactory());
  _curl_factory.reset(new CurlSessionFactory());
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
SessionFactory::~SessionFactory() {}

//------------------------------------------------------------------------------
// Get neon session factory
//------------------------------------------------------------------------------
NEONSessionFactory& SessionFactory::getNeon() {
  return *(_neon_factory.get());
}

//------------------------------------------------------------------------------
// Get curl session factory
//------------------------------------------------------------------------------
CurlSessionFactory& SessionFactory::getCurl() {
  return *(_curl_factory.get());
}

//------------------------------------------------------------------------------
// Set caching on or off
//------------------------------------------------------------------------------
void SessionFactory::setSessionCaching(bool caching) {
  _neon_factory->setSessionCaching(caching);
}

//------------------------------------------------------------------------------
// Get caching status
//------------------------------------------------------------------------------
bool SessionFactory::getSessionCaching() const {
  return _neon_factory->getSessionCaching();
}

//------------------------------------------------------------------------------
// "httpize" protocol
//------------------------------------------------------------------------------
std::string SessionFactory::httpizeProtocol(const std::string &protocol) {
    std::string proto = protocol;
    if(proto.compare(0,4, "http") == 0 ||
       proto.compare(0,2, "s3") == 0   ||
       proto.compare(0,3, "dav") == 0  ||
       proto.compare(0, 6, "gcloud") == 0 ||
       proto.compare(0, 5, "swift") == 0 ||
       proto.compare(0, 3, "cs3") == 0){

        proto.assign("http");

      if(*(protocol.rbegin()) == 's') {
          proto.append("s");
      }
    }

    return proto;
}

//------------------------------------------------------------------------------
// Create session key based on Uri
//------------------------------------------------------------------------------
std::string SessionFactory::makeSessionKey(const Uri &uri) {
    return SSTR(httpizeProtocol(uri.getProtocol()) << uri.getHost() << ":" << uri.getPort());
}


}
