/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#ifndef DAVIX_CURL_RESPONSE_BUFFER_HPP
#define DAVIX_CURL_RESPONSE_BUFFER_HPP

#include <vector>
#include <deque>
#include <stddef.h>

namespace Davix {

//------------------------------------------------------------------------------
// Utility class to buffer HTTP response body
//------------------------------------------------------------------------------
class ResponseBuffer {
public:

  ResponseBuffer(size_t bsize = 16384u);
  ~ResponseBuffer();

  //----------------------------------------------------------------------------
  // Feed len bytes into the buffer
  //----------------------------------------------------------------------------
  void feed(const char *buff, size_t len);

  //----------------------------------------------------------------------------
  // Consume a maximum of maxlen bytes out of the buffer
  //----------------------------------------------------------------------------
  size_t consume(char *target, size_t maxlen);

  //----------------------------------------------------------------------------
  // Check total stored bytes
  //----------------------------------------------------------------------------
  size_t size() const;

private:
  std::deque<std::vector<char>> buffers;
  size_t bufferSize;
  size_t posWrite;
  size_t posRead;
};

}

#endif

