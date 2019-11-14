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

#include "ResponseBuffer.hpp"
#include <string.h>
#include <iostream>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

namespace Davix {

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
ResponseBuffer::ResponseBuffer(size_t bsize) : bufferSize(bsize), posWrite(0),
    posRead(0) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
ResponseBuffer::~ResponseBuffer() {}

//------------------------------------------------------------------------------
// Feed len bytes into the buffer
//------------------------------------------------------------------------------
void ResponseBuffer::feed(const char *buff, size_t len) {
  size_t buffPos = 0;

  while(len > 0) {
    if(buffers.size() == 0 || posWrite == bufferSize) {
      buffers.emplace_back();
      buffers.back().resize(bufferSize);
      posWrite = 0;
    }

    size_t bytesToWrite = std::min(len, bufferSize - posWrite);

    ::memcpy(buffers.back().data()+posWrite, buff+buffPos, bytesToWrite);
    buffPos += bytesToWrite;
    len -= bytesToWrite;
    posWrite += bytesToWrite;
  }
}

//------------------------------------------------------------------------------
// Consume a maximum of maxlen bytes out of the buffer
//------------------------------------------------------------------------------
size_t ResponseBuffer::consume(char *target, size_t maxlen) {
  size_t bytesDelivered = 0;

  while(maxlen > 0) {
    if(buffers.size() == 0) {
      break;
    }

    if(buffers.size() == 1 && posRead >= posWrite) {
      break;
    }

    if(posRead == bufferSize) {
      buffers.pop_front();
      posRead = 0;
    }

    size_t bytesToCopy = 0;
    if(buffers.size() == 1) {
      bytesToCopy = posWrite - posRead;
    }
    else {
      bytesToCopy = bufferSize - posRead;
    }

    bytesToCopy = std::min(maxlen, bytesToCopy);
    ::memcpy(target+bytesDelivered, buffers.front().data()+posRead, bytesToCopy);

    posRead += bytesToCopy;
    maxlen -= bytesToCopy;
    bytesDelivered += bytesToCopy;
  }

  return bytesDelivered;
}

//------------------------------------------------------------------------------
// Check total stored bytes
//------------------------------------------------------------------------------
size_t ResponseBuffer::size() const {
  size_t total = 0;

  if(buffers.size() == 0) {
    return total;
  }

  total += bufferSize * buffers.size();

  total -= posRead;
  total -= (bufferSize - posWrite);
  return total;
}

}
