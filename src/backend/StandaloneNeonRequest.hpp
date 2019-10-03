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

#include "BoundHooks.hpp"
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
// compatibility hacks, no automatic retries, nothing. Just a single
// HTTP request / response.
//
// The magic is to be added by higher-level classes building upon this one.
//------------------------------------------------------------------------------
class StandaloneNeonRequest {
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
  StandaloneNeonRequest(NEONSessionFactory *sessionFactory, bool reuseSession,
    const BoundHooks &boundHooks, const Uri &uri, const RequestParams &param);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~StandaloneNeonRequest();


private:
  BoundHooks _bound_hooks;
  // std::unique_ptr<NeonSessionWrapper> _session;

};

}

#endif
