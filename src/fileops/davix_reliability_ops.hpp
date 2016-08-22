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

#ifndef METALINKOPS_HPP
#define METALINKOPS_HPP


#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davfile.hpp>
#include <fileops/httpiochain.hpp>

namespace Davix{

///
/// \brief The MetalinkOps class
///
///  Metalink chain element
///
///  the metalink chain element handle the recovery using metalink for any "reading" operation of the I/O chain
///
class MetalinkOps: public HttpIOChain{
public:
    MetalinkOps();
    virtual ~MetalinkOps();

    virtual StatInfo & statInfo(IOChainContext &iocontext, StatInfo &st_info);

    virtual dav_ssize_t read(IOChainContext & iocontext, void *buf, dav_size_t count);

    virtual dav_ssize_t pread(IOChainContext & iocontext, void* buf, dav_size_t count, dav_off_t offset);

    virtual dav_ssize_t preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // read to fd
    virtual dav_ssize_t readToFd(IOChainContext & iocontext, int fd, dav_size_t size);

    // calc replica
    virtual std::vector<File> & getReplicas(IOChainContext & iocontext, std::vector<File> & vec);


};



/// \brief The AutoRetry class
///
///  AutoRetry chain element
///
///  the Retry chain element handle the recovery using auto-retry for any "reading" operation of the I/O chain
///
class AutoRetryOps: public HttpIOChain{
public:
    AutoRetryOps();
    virtual ~AutoRetryOps();

    virtual StatInfo & statInfo(IOChainContext &iocontext, StatInfo &st_info);

    virtual dav_ssize_t read(IOChainContext & iocontext, void *buf, dav_size_t count);

    virtual dav_ssize_t pread(IOChainContext & iocontext, void* buf, dav_size_t count, dav_off_t offset);

    virtual dav_ssize_t preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // read to fd
    virtual dav_ssize_t readToFd(IOChainContext & iocontext, int fd, dav_size_t size);

};


// utilities
int davix_metalink_header_parser(const std::string & header_key, const std::string & header_value,
                                 const Uri & u_original,
                                 Uri & metalink);


}

#endif // METALINKOPS_HPP
