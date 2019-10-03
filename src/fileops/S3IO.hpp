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

struct DynafedUris {
  std::vector<std::string> chunks;
  std::string post;
};

class S3IO : public HttpIOChain {
public:
  S3IO();
  ~S3IO();

  // write from content provider
  virtual dav_ssize_t writeFromProvider(IOChainContext & iocontext, ContentProvider &provider);

  void performUgrS3MultiPart(IOChainContext & iocontext, const std::string &posturl, const std::string &pluginId, ContentProvider &provider, DavixError **err);

private:

  // Returns uploadId
  std::string initiateMultipart(IOChainContext & iocontext);
  std::string initiateMultipart(IOChainContext & iocontext, const Uri &url);

  DynafedUris retrieveDynafedUris(IOChainContext & iocontext, const std::string &uploadId, const std::string &pluginId, size_t nchunks);

  // Given the upload id, write the given chunk. Return object ETag,
  // necessary to commit upload.
  std::string writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const std::string &uploadId, int partNumber);
  std::string writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, const Uri &uri, int partNumber);


  // Given upload id and last chunk, commit chunks
  void commitChunks(IOChainContext & iocontext,  const std::string &uploadId, const std::vector<std::string> &etags);
  void commitChunks(IOChainContext & iocontext,  const Uri &uri, const std::vector<std::string> &etags);
};

}

#endif
