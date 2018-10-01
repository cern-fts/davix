/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2018
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

#ifndef S3_IO_HPP
#define S3_IO_HPP

#include <fileops/httpiochain.hpp>

namespace Davix{

class S3IO : public HttpIOChain {
public:
  S3IO();
  ~S3IO();

  // write the entire content from a file descriptor
  virtual dav_ssize_t writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size);

  // wirte the entire content from a defined callback
  virtual dav_ssize_t writeFromCb(IOChainContext & iocontext, const DataProviderFun & func, dav_size_t size);
private:
  // Returns uploadId
  std::string initiateMultipart(IOChainContext & iocontext);

  // Given the upload id, write the given chunk. Return object ETag,
  // necessary to commit upload.
  std::string writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const std::string &uploadId, int partNumber);

  // Given upload id and last chunk, commit chunks
  void commitChunks(IOChainContext & iocontext,  const std::string &uploadId, const std::vector<std::string> &etags);
};

}

#endif
