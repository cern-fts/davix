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

#ifndef DAVIX_CORE_CONTENT_PROVIDER_HPP
#define DAVIX_CORE_CONTENT_PROVIDER_HPP

#include "stdlib.h"

namespace Davix {

//------------------------------------------------------------------------------
// Abstract ContentProvider interface to provide the raw bytes for HTTP body
// content.
//
// Core methods:
// - pullBytes for reading through the contents.
//------------------------------------------------------------------------------
class ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Empty constructor
  //----------------------------------------------------------------------------
  ContentProvider() {}

  //----------------------------------------------------------------------------
  // Virtual destructor
  //----------------------------------------------------------------------------
  virtual ~ContentProvider() {}

  //----------------------------------------------------------------------------
  // Pull the specified number of bytes, write them onto the given buffer.
  //
  // Return value:
  // - Positive: The specified number of bytes were written onto the buffer.
  //
  // - Zero: We've reached EOF, no more bytes for you.
  //
  // - Negative: An unexpected error happened, and we are unable to continue.
  //   The returned value is the negative of the errno. Call errc() and
  //   errMsg() for more information about the issue.
  //----------------------------------------------------------------------------
  virtual size_t pullBytes(char* target, size_t requestedBytes) = 0;

  //----------------------------------------------------------------------------
  // Rewind the provider to the beginning. We're probably doing a redirect or
  // something, and need to access the contents from the beginning once again.
  //
  // Return false if, for some reason, this is not possible. The request will
  // fail.
  //----------------------------------------------------------------------------
  virtual bool rewind() = 0;

  //----------------------------------------------------------------------------
  // Get total size - should return a constant throughout the lifetime of this
  // object.
  //
  // Return -1 if size is not known beforehand. davix does not currently\
  // support this option with libneon, as we don't have chunked encoding
  // for uploads yet.
  //----------------------------------------------------------------------------
  virtual size_t getSize() = 0;
};

}

#endif

