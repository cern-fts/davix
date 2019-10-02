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

#include "ContentProvider.hpp"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace Davix {

//------------------------------------------------------------------------------
// Empty constructor
//------------------------------------------------------------------------------
ContentProvider::ContentProvider() {
  _errc = 0;
}

//------------------------------------------------------------------------------
// Is the object ok?
//------------------------------------------------------------------------------
bool ContentProvider::ok() const {
  return (_errc == 0);
}

//------------------------------------------------------------------------------
// Has there been an error?
//------------------------------------------------------------------------------
int ContentProvider::getErrc() const {
  return _errc;
}

//------------------------------------------------------------------------------
// Get error message
//------------------------------------------------------------------------------
std::string ContentProvider::getError() const {
  return _errMsg;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
BufferContentProvider::BufferContentProvider(const char* buf, size_t count)
: _buffer(buf), _count(count), _pos(0) {}

//------------------------------------------------------------------------------
// pullBytes implementation.
//------------------------------------------------------------------------------
ssize_t BufferContentProvider::pullBytes(char* target, size_t requestedBytes) {
  if(_pos >= _count) {
    // EOF
    return 0;
  }

  size_t bytesToGive = requestedBytes;
  if(_pos + bytesToGive > _count) {
    // Asked for more bytes than we have, just give all remaining ones
    bytesToGive = _count - _pos;
  }

  ::memcpy(target, _buffer + _pos, bytesToGive);
  _pos += bytesToGive;
  return bytesToGive;
}

//------------------------------------------------------------------------------
// Rewind implementation.
//------------------------------------------------------------------------------
bool BufferContentProvider::rewind() {
  _pos = 0;
  return true;
}

//------------------------------------------------------------------------------
// getSize implementation.
//------------------------------------------------------------------------------
ssize_t BufferContentProvider::getSize() {
  return _count;
}

//------------------------------------------------------------------------------
// FdContentProvider constructor
//------------------------------------------------------------------------------
FdContentProvider::FdContentProvider(int fd) : _fd(fd) {
  _fd_size = ::lseek(_fd, 0, SEEK_END);

  if(_fd_size == -1) {
    _errc = errno;
    _errMsg = strerror(_errc);
  }
  else {
    rewind();
  }
}

//------------------------------------------------------------------------------
// pullBytes implementation.
//------------------------------------------------------------------------------
ssize_t FdContentProvider::pullBytes(char* target, size_t requestedBytes) {
  if(!ok()) {
    return - _errc;
  }

  while(true) {
    ssize_t retval = ::read(_fd, target, requestedBytes);

    if(retval >= 0) {
      // No errors
      return retval;
    }
    else if(retval == -1 && errno == EINTR) {
      // Interrupted by a signal... retry...
      continue;
    }
    else {
      // Error
      _errc = errno;
      _errMsg = strerror(_errc);
      return - _errc;
    }
  }
}

//------------------------------------------------------------------------------
// Rewind implementation.
//------------------------------------------------------------------------------
bool FdContentProvider::rewind() {
  if(!ok()) {
    return false;
  }

  off_t retval = ::lseek(_fd, 0, SEEK_SET);
  if(retval == -1) {
    _errc = errno;
    _errMsg = strerror(_errc);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// getSize implementation.
//------------------------------------------------------------------------------
ssize_t FdContentProvider::getSize() {
  return _fd_size;
}


}
