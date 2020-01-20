/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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

#ifndef HTTPIOVEC_HPP
#define HTTPIOVEC_HPP

#include <davix.hpp>
#include <fileops/iobuffmap.hpp>
#include <fileops/httpiochain.hpp>
#include "libs/IntervalTree.h"

namespace Davix{

// simple chunk info handler
struct ChunkInfo {
    ChunkInfo() :
        offset(0),
        size(0),
        bounded(false){}
    dav_off_t offset;
    dav_size_t size;
    bool bounded;
};

struct ElemChunk{
    ElemChunk(const DavIOVecInput* in, DavIOVecOuput* ou) :
        _in(in),
        _ou(ou),
        _cursor((char*) in->diov_buffer){
        _ou->diov_size=0; // reset elem read status
        _ou->diov_buffer = _in->diov_buffer;
    }

    const DavIOVecInput *_in;
    DavIOVecOuput * _ou;
    char *_cursor;

};

typedef std::multimap<dav_off_t, ElemChunk> MapChunk;
typedef std::vector<std::pair<dav_off_t, dav_size_t> > SortedRanges;

struct MultirangeResult {
    enum OperationResult { SUCCESS, NOMULTIRANGE, SUCCESS_BUT_NO_MULTIRANGE };
    OperationResult res;
    dav_ssize_t size_bytes;

    MultirangeResult(OperationResult _res, dav_ssize_t _size_bytes)
        : res(_res), size_bytes(_size_bytes) {}
};

////
/// \brief The HttpIOVecOps class
///
///  the HttpIOVecOps io vec chain element handle all operations relative to Vector requests
class HttpIOVecOps : public HttpIOChain
{
public:
    HttpIOVecOps(){}
    virtual ~HttpIOVecOps(){}

    dav_ssize_t preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // these two should have been private, but because of pthread we need to call them from
    // a function
    dav_ssize_t singleRangeRequest(IOChainContext & iocontext,
                                   const DavIOVecInput * input,
                                   DavIOVecOuput * output);

    dav_ssize_t singleRangeRequest(IOChainContext & iocontext,
                                   const IntervalTree<ElemChunk> & tree,
                                   dav_off_t offset, dav_size_t size);

private:

    MultirangeResult performMultirange(IOChainContext & iocontext,
                                       const IntervalTree<ElemChunk> &tree,
                                       const SortedRanges & ranges);

    dav_ssize_t simulateMultirange(IOChainContext & iocontext,
                                   const IntervalTree<ElemChunk> & tree,
                                   const SortedRanges & ranges,
                                   uint nconnections);

    dav_ssize_t parseMultipartRequest(HttpRequest & req,
                                      const IntervalTree<ElemChunk> & tree,
                                      DavixError** tmp_err);

    dav_ssize_t simulateMultiPartRequest(HttpRequest & _req,
                                         const IntervalTree<ElemChunk> & tree,
                                         DavixError** err);
};


int find_header_params(char* buffer, dav_size_t buffer_len, dav_size_t* part_size, dav_off_t* part_offset);

int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err);


int parse_multi_part_header(HttpRequest& req, const std::string & boundary, ChunkInfo & info,
                            int & n_try, DavixError** err);

void HttpIoVecSetupErrorMultiPart(DavixError** err);

} // Davix

#endif // HTTPIOVEC_HPP
