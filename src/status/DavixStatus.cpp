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

#include <status/DavixStatus.hpp>

namespace Davix {

//------------------------------------------------------------------------------
// Empty constructor - OK status, with empty error message, and empty scope
//------------------------------------------------------------------------------
Status::Status() {
  d_ptr = NULL;
}

//------------------------------------------------------------------------------
// Constructor - specify scope, errno, and error message
//------------------------------------------------------------------------------
Status::Status(const std::string &scope, StatusCode::Code errcode, const std::string& errmsg) {
  d_ptr = new DavixErrorInternal(scope, errcode, errmsg);
}

//------------------------------------------------------------------------------
// Constructor from DavixError**
//------------------------------------------------------------------------------
Status::Status(DavixError** err) {
  if(err && *err) {
    d_ptr = new DavixErrorInternal( (*err)->getErrScope(), (*err)->getStatus(), (*err)->getErrMsg());
  }
  else {
    d_ptr = NULL;
  }
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
Status::~Status() {
  clear();
}

//------------------------------------------------------------------------------
// Copy constructor
//------------------------------------------------------------------------------
Status::Status(const Status &other) {
  d_ptr = NULL;
  *this = other;
}

//------------------------------------------------------------------------------
// Copy assignment operator
//------------------------------------------------------------------------------
Status& Status::operator=(const Status &other) {
  clear();

  if(other.d_ptr) {
    d_ptr = new DavixErrorInternal(*other.d_ptr);
  }
  else {
    d_ptr = NULL;
  }

  return *this;
}

//------------------------------------------------------------------------------
// Check if status indicates any error
//------------------------------------------------------------------------------
bool Status::ok() const {
  return getCode() == StatusCode::OK;
}

//------------------------------------------------------------------------------
// Check if status indicates any error - as integer, 0 for OK, -1 for not-ok
//------------------------------------------------------------------------------
bool Status::okAsInt() const {
  if(ok()) {
    return 0;
  }

  return -1;
}

//------------------------------------------------------------------------------
// Get status code
//------------------------------------------------------------------------------
StatusCode::Code Status::getCode() const {
  if(!d_ptr) {
    return StatusCode::OK;
  }
  else {
    return d_ptr->_code;
  }
}

//------------------------------------------------------------------------------
// Get error message
//------------------------------------------------------------------------------
std::string Status::getErrorMessage() const {
  if(!d_ptr) {
    return std::string();
  }
  else {
    return d_ptr->_errMsg;
  }
}

//------------------------------------------------------------------------------
// Get scope
//------------------------------------------------------------------------------
std::string Status::getScope() const {
  if(!d_ptr) {
    return std::string();
  }
  else {
    return d_ptr->_scope;
  }
}

//------------------------------------------------------------------------------
// Clear contents
//------------------------------------------------------------------------------
void Status::clear() {
  if(d_ptr) {
    delete d_ptr;
    d_ptr = NULL;
  }
}

//------------------------------------------------------------------------------
// To DavixError (ugh)
//------------------------------------------------------------------------------
int Status::toDavixError(DavixError **err) const {
  if(d_ptr) {
    DavixError::setupError(err, d_ptr->_scope, d_ptr->_code, d_ptr->_errMsg);
    return 1;
  }
  else {
    DavixError::clearError(err);
    return 0;
  }
}

}