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

#include "BackendRequest.hpp"
#include <string>

namespace Davix {

//------------------------------------------------------------------------------
// Default constructor
//------------------------------------------------------------------------------
BackendRequest::BackendRequest(const Uri &uri)
  : _current( new Uri(uri)),
    _orig(_current),
    _params(),
    _request_type("GET"),
    _req_flag(RequestFlag::IdempotentRequest),
    _content_ptr(),
    _content_len(0),
    _content_offset(0),
    _content_body(),
    _fd_content(-1),
    _content_provider() {}


//------------------------------------------------------------------------------
// Several different ways to provide the request body.
//------------------------------------------------------------------------------
void BackendRequest::setRequestBody(const std::string & body) {
  _content_body = std::string(body);
  _content_ptr = (char*) _content_body.c_str();
  _content_len = _content_body.size();
  _fd_content = -1;
}

void BackendRequest::setRequestBody(const void * buffer, dav_size_t len) {
  _content_ptr = (char*) buffer;
  _content_len = len;
  _fd_content = -1;
}

void BackendRequest::setRequestBody(int fd, dav_off_t offset, dav_size_t len) {
  _fd_content = fd;
  _content_ptr = NULL;
  _content_len = len;
  _content_offset = offset;
}

void BackendRequest::setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata) {
  _content_len  = len;
  _content_provider.callback = provider;
  _content_provider.udata = udata;
}

//------------------------------------------------------------------------------
// Set request parameters.
//------------------------------------------------------------------------------
void BackendRequest::setParameters(const RequestParams &p) {
  _params = p;
}

//------------------------------------------------------------------------------
// Get request parameters.
//------------------------------------------------------------------------------
RequestParams& BackendRequest::getParameters() {
  return _params;
}

}
