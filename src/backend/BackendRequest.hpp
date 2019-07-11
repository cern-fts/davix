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

#ifndef BACKEND_REQUEST_HPP
#define BACKEND_REQUEST_HPP

namespace Davix{

//------------------------------------------------------------------------------
// Abstract HTTP request type towards a backend.
//------------------------------------------------------------------------------
class BackendRequest {
public:
  //----------------------------------------------------------------------------
  // Default constructor
  //----------------------------------------------------------------------------
  BackendRequest()
  : _request_type("GET"),
    _req_flag(RequestFlag::IdempotentRequest) {}

  //----------------------------------------------------------------------------
  // Virtual destructor
  //----------------------------------------------------------------------------
  virtual ~BackendRequest() {}

  //----------------------------------------------------------------------------
  // No evil constructors, no move semantics.
  //----------------------------------------------------------------------------
  BackendRequest(const BackendRequest& other) = delete;
  BackendRequest(BackendRequest&& other) = delete;
  BackendRequest& operator=(BackendRequest&& other) = delete;
  BackendRequest& operator=(const BackendRequest& other) = delete;

  //----------------------------------------------------------------------------
  // Add custom header to the request, replace an existing one if already
  // exists. If value is empty, the entire header line is removed.
  //----------------------------------------------------------------------------
  void addHeaderField(const std::string & field, const std::string & value){
    _headers_field.emplace_back(field, value);
  }

  //----------------------------------------------------------------------------
  // Set the request verb. (such as GET, POST, PUT, PROPFIND, etc)
  //----------------------------------------------------------------------------
  void setRequestMethod(const std::string &val){
    _request_type = val;
  }

  //----------------------------------------------------------------------------
  // Get the request verb. (such as GET, POST, PUT, PROPFIND, etc)
  //----------------------------------------------------------------------------
  std::string getRequestMethod() {
    return _request_type;
  }

  //----------------------------------------------------------------------------
  // Set the value of a request flag
  //----------------------------------------------------------------------------
  void setFlag(const RequestFlag::RequestFlag flag, bool value) {
    if(value) {
      _req_flag |= flag;
    }
    else {
      _req_flag &= ~(flag);
    }
  }

  //----------------------------------------------------------------------------
  // Get the value of a request flag
  //----------------------------------------------------------------------------
  bool getFlag(const RequestFlag::RequestFlag flag) const {
    return _req_flag & ((int) flag);
  }

protected:
  //----------------------------------------------------------------------------
  // Member variables common to all implementations.
  //----------------------------------------------------------------------------
  std::vector<std::pair<std::string, std::string>> _headers_field;
  std::string _request_type;
  int _req_flag;

};


}


#endif
