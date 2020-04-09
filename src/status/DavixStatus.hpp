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

#ifndef DAVIX_STATUS_OBJECT_HPP
#define DAVIX_STATUS_OBJECT_HPP

#include <status/DavixErrorInternal.hpp>

namespace Davix {

class Status {
public:
  //----------------------------------------------------------------------------
  // Empty constructor - OK status, with empty error message, and empty scope
  //----------------------------------------------------------------------------
  Status();

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  ~Status();

  //----------------------------------------------------------------------------
  // Copy constructor
  //----------------------------------------------------------------------------
  Status(const Status &other);

  //----------------------------------------------------------------------------
  // Copy assignment operator
  //----------------------------------------------------------------------------
  Status& operator=(const Status &other);

  //----------------------------------------------------------------------------
  // Constructor - specify scope, errno, and error message
  //----------------------------------------------------------------------------
  Status(const std::string &scope, StatusCode::Code errcode, const std::string& errmsg);

  //----------------------------------------------------------------------------
  // Constructor from DavixError**
  //----------------------------------------------------------------------------
  Status(DavixError** err);

  //----------------------------------------------------------------------------
  // Check if status indicates any error
  //----------------------------------------------------------------------------
  bool ok() const;

  //----------------------------------------------------------------------------
  // Check if status indicates any error - as integer, 0 for OK, -1 for not-ok
  //----------------------------------------------------------------------------
  bool okAsInt() const;

  //----------------------------------------------------------------------------
  // Get status code
  //----------------------------------------------------------------------------
  StatusCode::Code getCode() const;

  //----------------------------------------------------------------------------
  // Get error message
  //----------------------------------------------------------------------------
  std::string getErrorMessage() const;

  //----------------------------------------------------------------------------
  // Get scope
  //----------------------------------------------------------------------------
  std::string getScope() const;

  //----------------------------------------------------------------------------
  // Clear contents
  //----------------------------------------------------------------------------
  void clear();

  //----------------------------------------------------------------------------
  // To DavixError (ugh)
  //----------------------------------------------------------------------------
  int toDavixError(DavixError **err) const;

private:
  DavixErrorInternal* d_ptr;
};

}

#endif
