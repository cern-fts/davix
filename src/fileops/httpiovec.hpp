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

private:
    dav_ssize_t singleRangeRequest(IOChainContext & iocontext,
                                   const DavIOVecInput * input,
                                   DavIOVecOuput * output);

    MultirangeResult performMultirange(IOChainContext & iocontext,
                              const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    dav_ssize_t simulateMultirange(IOChainContext & iocontext,
                              const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    dav_ssize_t readPartialBufferVecRequest(HttpRequest & req,
                              const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err);

    dav_ssize_t parseMultipartRequest(HttpRequest & req,
                                      const DavIOVecInput *input_vec,
                                      DavIOVecOuput * output_vec,
                                      const dav_size_t count_vec, DavixError** tmp_err);

    dav_ssize_t simulateMultiPartRequest(HttpRequest & req,
                                         const DavIOVecInput *input_vec,
                                         DavIOVecOuput * output_vec,
                                         const dav_size_t count_vec, DavixError** tmp_err);

};



bool is_a_start_boundary_part(char* buffer, dav_size_t s_buff, const std::string & boundary,
                            DavixError** err);

int find_header_params(char* buffer, dav_size_t buffer_len, dav_size_t* part_size, dav_off_t* part_offset);

int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err);


int parse_multi_part_header(HttpRequest& req, const std::string & boundary, ChunkInfo & info,
                            int & n_try, DavixError** err);

int davIOVecProvider(const DavIOVecInput *input_vec, dav_ssize_t & counter, dav_ssize_t number, dav_off_t & begin, dav_off_t & end);




void HttpIoVecSetupErrorMultiPart(DavixError** err);

} // Davix

#endif // HTTPIOVEC_HPP
