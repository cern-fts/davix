/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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


#ifndef DAVIX_HOOKS_HPP
#define DAVIX_HOOKS_HPP

#include <request/httprequest.hpp>

namespace Davix{

class HttpRequest;
class RequestParams;
class Uri;

#define DAVIX_HOOK_REQUEST_PRE_RUN 0x00
#define DAVIX_HOOK_REQUEST_PRE_SEND 0x01
#define DAVIX_HOOK_REQUEST_PRE_RECVE 0x02
#define DAVIX_HOOk_REQUEST_NUM  0x04

/// Hook called before processing any request
typedef void (*hookRequestPreRun)(RequestParams& p, HttpRequest & req, Uri & u, void* userdata);

/// Hook called when sending any request, just after sending headers
typedef void (*hookRequestPreSend)(HttpRequest& req, const std::string & start_line, void* userdate);

/// Hook called when receiving an request, just after receiving headers
typedef void (*hookRequestPreRece)(HttpRequest& req, const std::string & start_line, void* userdata);

}


#endif // DAVIX_HOOKS_HPP
