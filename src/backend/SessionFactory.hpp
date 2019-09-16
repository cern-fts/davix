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

#include <atomic>

namespace Davix {

//------------------------------------------------------------------------------
// Abstract SessionFactory class towards a type of backend.
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
  // Set caching on or off
  //----------------------------------------------------------------------------
  void setSessionCaching(bool caching);

  //----------------------------------------------------------------------------
  // Get caching status
  //----------------------------------------------------------------------------
  bool getSessionCaching() const;

protected:
  std::atomic<bool> _session_caching;

};

}

#endif

