/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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


#ifndef DAVIX_HOOKS_FWD_HPP
#define DAVIX_HOOKS_FWD_HPP

#include <utils/davix_types.hpp>
#include <request/httprequest.hpp>


/**
  @file davix_hooks_fwd.hpp
  @author Devresse Adrien
  @brief Hook definitions of davix
 */



namespace Davix{

class HttpRequest;
class RequestParams;
class Uri;


#ifdef __DAVIX_HAS_STD_FUNCTION

/// Hook called before processing any request
typedef std::function< void (RequestParams& p, HttpRequest & req, Uri & u) > RequestPreRunHook;

/// Hook called when sending any request, just after sending headers
typedef std::function<void (HttpRequest& req, const std::string & start_line) > RequestPreSendHook;

/// Hook called when receiving any request, just after receiving headers
typedef std::function<void (HttpRequest& req, const std::string & init_line, const HeaderVec & headers, int status_code) > RequestPreReceHook;


#endif

}


#endif // DAVIX_HOOKS_FWD_HPP
