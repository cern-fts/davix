/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2021
 * Author: Shiting Long <s.long@fz-juelich.de> (Forschungszentrum Juelich)
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

#ifndef DAVIX_SWIFTIO_HPP
#define DAVIX_SWIFTIO_HPP

#include <fileops/httpiochain.hpp>

namespace Davix{

typedef std::pair <std::string, int> Prop;

class SwiftIO : public HttpIOChain {
public:
    SwiftIO();
    ~SwiftIO();

    // write from content provider
    virtual dav_ssize_t writeFromProvider(IOChainContext & iocontext, ContentProvider &provider);

private:

    // Write the given chunk. Return object properties (etag + size_bytes),
    // necessary to commit upload.
    std::string writeChunk(IOChainContext & iocontext, const char* buff, dav_size_t size, int partNumber);


    // Given the properties (etag + size_bytes) of the segments, commit chunks
    void commitChunks(IOChainContext & iocontext, const std::vector<Prop> &props);
    // Commit chunks in a inline manner if the number of segments has exceeded the limit defined by max_manifest_segments
    void commitInlineChunks(IOChainContext & iocontext, const std::vector<Prop> &props, const size_t MaxManifestSegments);
};

}

#endif //DAVIX_SWIFTIO_HPP
