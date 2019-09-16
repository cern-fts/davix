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
SessionFactory::SessionFactory() {
  _session_caching = !isSessionCachingDisabled();
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
SessionFactory::~SessionFactory() {}

//------------------------------------------------------------------------------
// Set caching on or off
//------------------------------------------------------------------------------
void SessionFactory::setSessionCaching(bool caching) {
  std::lock_guard<std::mutex> lock(_session_caching_mtx);
  _session_caching = caching && !isSessionCachingDisabled();
}

//------------------------------------------------------------------------------
// Get caching status
//------------------------------------------------------------------------------
bool SessionFactory::getSessionCaching() const {
  std::lock_guard<std::mutex> lock(_session_caching_mtx);
  return _session_caching;
}


}
