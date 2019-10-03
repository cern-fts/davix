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

#include <request/httprequest.hpp>
#include "stdlib.h"
#include <string>

namespace Davix {

//------------------------------------------------------------------------------
// Abstract ContentProvider interface to provide the raw bytes for HTTP body
// content.
//
// Core methods:
// - pullBytes for reading through the contents.
// - rewind for starting back from the beginning.
// - getSize for getting the total size.
//------------------------------------------------------------------------------
class ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Empty constructor
  //----------------------------------------------------------------------------
  ContentProvider();

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
  virtual ssize_t pullBytes(char* target, size_t requestedBytes) = 0;

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
  // Return -1 if size is not known beforehand. davix does not currently
  // support this option with libneon, as we don't have chunked encoding
  // for uploads yet.
  //----------------------------------------------------------------------------
  virtual ssize_t getSize() = 0;

  //----------------------------------------------------------------------------
  // Is the object ok?
  //----------------------------------------------------------------------------
  bool ok() const;

  //----------------------------------------------------------------------------
  // Has there been an error?
  //----------------------------------------------------------------------------
  int getErrc() const;

  //----------------------------------------------------------------------------
  // Get error message
  //----------------------------------------------------------------------------
  std::string getError() const;

protected:
  int _errc;
  std::string _errMsg;
};

//------------------------------------------------------------------------------
// Content provider based on a simple buffer. No buffer ownership.
//------------------------------------------------------------------------------
class BufferContentProvider : public ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  BufferContentProvider(const char* buf, size_t count);

  //----------------------------------------------------------------------------
  // pullBytes implementation.
  //----------------------------------------------------------------------------
  ssize_t pullBytes(char* target, size_t requestedBytes);

  //----------------------------------------------------------------------------
  // Rewind implementation.
  //----------------------------------------------------------------------------
  bool rewind();

  //----------------------------------------------------------------------------
  // getSize implementation.
  //----------------------------------------------------------------------------
  ssize_t getSize();

private:
  const char* _buffer;
  size_t _count;
  size_t _pos;
};

//------------------------------------------------------------------------------
// Content provider based on an owned internal buffer - contents are copied
// on construction.
//------------------------------------------------------------------------------
class OwnedBufferContentProvider : public ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  OwnedBufferContentProvider(const char* buf, size_t count);

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  OwnedBufferContentProvider(const std::string &str);

  //----------------------------------------------------------------------------
  // pullBytes implementation.
  //----------------------------------------------------------------------------
  ssize_t pullBytes(char* target, size_t requestedBytes);

  //----------------------------------------------------------------------------
  // Rewind implementation.
  //----------------------------------------------------------------------------
  bool rewind();

  //----------------------------------------------------------------------------
  // getSize implementation.
  //----------------------------------------------------------------------------
  ssize_t getSize();

private:
  std::string _contents;
  BufferContentProvider _provider;
};

//------------------------------------------------------------------------------
// Content provider based on a file descriptor - no ownership on underlying
// file descriptor, keep open while this object is alive.
//------------------------------------------------------------------------------
class FdContentProvider : public ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Constructor. Start from the given offset, read a maximum of maxLen further
  // bytes. With maxLen = 0, read the entire rest of the fd contents.
  //----------------------------------------------------------------------------
  FdContentProvider(int fd, off_t offset = 0, size_t maxLen = 0);

  //----------------------------------------------------------------------------
  // pullBytes implementation.
  //----------------------------------------------------------------------------
  ssize_t pullBytes(char* target, size_t requestedBytes);

  //----------------------------------------------------------------------------
  // Rewind implementation.
  //----------------------------------------------------------------------------
  bool rewind();

  //----------------------------------------------------------------------------
  // getSize implementation.
  //----------------------------------------------------------------------------
  ssize_t getSize();

private:
  int _fd;
  ssize_t _fd_size;
  off_t _offset;
  size_t _target_len;
  bool eof;
  size_t _bytes_provided;
};

//------------------------------------------------------------------------------
// Content provider based on a HttpBodyProvider callback.
//------------------------------------------------------------------------------
class CallbackContentProvider : public ContentProvider {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  CallbackContentProvider(HttpBodyProvider provider, dav_size_t len,
    void *udata);

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  CallbackContentProvider(DataProviderFun provider, dav_size_t len);

  //----------------------------------------------------------------------------
  // pullBytes implementation.
  //----------------------------------------------------------------------------
  ssize_t pullBytes(char* target, size_t requestedBytes);

  //----------------------------------------------------------------------------
  // Rewind implementation.
  //----------------------------------------------------------------------------
  bool rewind();

  //----------------------------------------------------------------------------
  // getSize implementation.
  //----------------------------------------------------------------------------
  ssize_t getSize();


private:
  HttpBodyProvider _provider;
  DataProviderFun  _providerFun;

  dav_size_t _len;
  void *_udata;
};

}

#endif

