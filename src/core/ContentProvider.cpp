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
#include <sstream>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

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
// Constructor
//------------------------------------------------------------------------------
OwnedBufferContentProvider::OwnedBufferContentProvider(const char* buf, size_t count)
: _provider(NULL, 0) {

  _contents.resize(count);
  ::memcpy( (void*) _contents.data(), buf, count);
  _provider = BufferContentProvider(_contents.c_str(), _contents.size());
}

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
OwnedBufferContentProvider::OwnedBufferContentProvider(const std::string &str)
: _provider(NULL, 0) {

  _contents = str;
  _provider = BufferContentProvider(_contents.c_str(), _contents.size());
}

//------------------------------------------------------------------------------
// pullBytes implementation.
//------------------------------------------------------------------------------
ssize_t OwnedBufferContentProvider::pullBytes(char* target, size_t requestedBytes) {
  return _provider.pullBytes(target, requestedBytes);
}

//------------------------------------------------------------------------------
// Rewind implementation.
//------------------------------------------------------------------------------
bool OwnedBufferContentProvider::rewind() {
  return _provider.rewind();
}

//------------------------------------------------------------------------------
// getSize implementation.
//------------------------------------------------------------------------------
ssize_t OwnedBufferContentProvider::getSize() {
  return _provider.getSize();
}

//------------------------------------------------------------------------------
// FdContentProvider constructor
//------------------------------------------------------------------------------
FdContentProvider::FdContentProvider(int fd, off_t offset, size_t maxLen)
: _fd(fd), _offset(offset), _target_len(maxLen) {

  _fd_size = ::lseek(_fd, 0, SEEK_END);

  if(_offset >= _fd_size) {
    _errc = ERANGE;
    _errMsg = SSTR("Invalid offset (" << offset << ") given, fd contains only " << _fd_size << " bytes");
    return;
  }

  if(_target_len == 0) {
    _target_len = _fd_size - _offset;
  }
  else {
    _target_len = std::min<ssize_t>( (ssize_t) _target_len, (ssize_t) _fd_size - _offset);
  }

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

  if(eof) {
    return 0;
  }

  if(requestedBytes > _target_len - _bytes_provided) {
    requestedBytes = _target_len - _bytes_provided;
  }

  while(true) {
    ssize_t retval = ::read(_fd, target, requestedBytes);

    if(retval >= 0) {
      // No errors
      _bytes_provided += retval;
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

  _bytes_provided = 0;
  eof = false;

  off_t retval = ::lseek(_fd, _offset, SEEK_SET);
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
  return _target_len;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
CallbackContentProvider::CallbackContentProvider(HttpBodyProvider provider, dav_size_t len,
  void *udata) : _provider(provider), _len(len), _udata(udata) {}

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CallbackContentProvider::CallbackContentProvider(DataProviderFun provider, dav_size_t len)
: _providerFun(provider), _len(len) {}

//------------------------------------------------------------------------------
// pullBytes implementation.
//------------------------------------------------------------------------------
ssize_t CallbackContentProvider::pullBytes(char* target, size_t requestedBytes) {
  if(!ok()) {
    return - _errc;
  }

  if(requestedBytes == 0) {
    return 0;
  }

  ssize_t retval;
  if(_providerFun) {
    retval = _providerFun(target, requestedBytes);
  }
  else {
    retval = _provider(_udata, target, requestedBytes);
  }

  if(retval < 0) {
    _errc = -retval;
    _errMsg = strerror(_errc);
    return -_errc;
  }

  return retval;
}

//------------------------------------------------------------------------------
// Rewind implementation.
//------------------------------------------------------------------------------
bool CallbackContentProvider::rewind() {
  if(!ok()) {
    return false;
  }

  if(_providerFun) {
    _providerFun(NULL, 0);
  }
  else {
    _provider(_udata, NULL, 0);
  }

  return true;
}

//------------------------------------------------------------------------------
// getSize implementation.
//------------------------------------------------------------------------------
ssize_t CallbackContentProvider::getSize() {
  return _len;
}

}
