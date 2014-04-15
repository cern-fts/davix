/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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

#pragma once
#ifndef DAVIX_IOBUFFMAP_HPP
#define DAVIX_IOBUFFMAP_HPP

#include <davix_internal.hpp>
#include <fileops/fileutils.hpp>



namespace Davix {

//
// Internal POSIX like to HTTP RW operation mapper
// provides facilities for caching
//


/// RW operation mapped to pure HTTP ops
class HttpIO{
public:
    HttpIO(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~HttpIO();

    // full sequential read of a file from begining to the end
    dav_ssize_t readFullBuff(void* buf, dav_size_t count, DavixError** err);

    // read to dynamically allocated buffer
    dav_ssize_t readFull(std::vector<char> & buffer, DavixError** err);

    // read to dynamically allocated buffer
    dav_ssize_t readFull(std::string & buffer , DavixError** err);


    // execute a plain HTTP stat method for file info
    int stat(struct stat* st, DavixError** err);

    //
    void resetFullRead();

    // position independant read operation,
    // similar to pread except that does not need open() before
    dav_ssize_t readPartialBuffer(void* buf, dav_size_t count, dav_off_t offset, DavixError** err);

    // vec read
    dav_ssize_t readPartialBufferVec(const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          const dav_size_t count_vec, DavixError** err);

    // read to fd
    dav_ssize_t readToFd(int fd, dav_size_t size, DavixError** err);

    // position independant write operation,
    // similar to pwrite do not need open() before
    dav_ssize_t writeFullFromFd(int fd, dav_size_t size, DavixError** err);

protected:
    Context & _c;
    Uri _uri;
    RequestParams _params;
    boost::mutex _rwlock;

    dav_off_t _read_pos; //curent read file offset
    bool _read_endfile;

private:
    HttpRequest * _read_req;


    HttpIO(const HttpIO & );
    HttpIO & operator=(const HttpIO & );


};

///
/// RW operation with buffering support and POSIX like interface
class HttpIOBuffer : public HttpIO{
public:
    HttpIOBuffer(Context & c, const Uri & uri, const RequestParams * params);
    virtual ~HttpIOBuffer();

    // open the file associated with the davix IOBuffMap
    // do a simple check if the file exist and try to anticipate the next ops
    bool open(int flags, DavixError** err);

    //
    dav_ssize_t read(void* buf, dav_size_t count, DavixError** err);

    dav_ssize_t pread(void* buf, dav_size_t count, dav_off_t offset, DavixError** err);

    dav_ssize_t preadVec(const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          dav_size_t count_vec, DavixError** err);


    // give information on the future operation for prefecting
    void prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv);

    //
    dav_ssize_t write(const void* buf, dav_size_t count, DavixError** err);


    //
    dav_ssize_t pwrite(const void* buf, dav_size_t count, dav_off_t offset, DavixError** err);


    //
    dav_off_t lseek(dav_off_t offset, int flags, DavixError** err);

    // commit any pending operation on the file descriptor
    int commit(DavixError** err);

protected:

    dav_size_t _file_size;
    bool _file_exist;
    dav_off_t _pos;
    bool _opened;
    advise_t _last_advise;

private:

    inline bool isAdviseFullRead(){
        return (_last_advise == AdviseAuto || _last_advise == AdviseSequential);
    }

    HttpIOBuffer(const HttpIOBuffer & );
    HttpIOBuffer & operator=(const HttpIOBuffer & );
};

// create a single-ton filestream for cache from
int get_valid_cache_file(FILE** stream, DavixError** err);


dav_ssize_t read_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read,  dav_off_t off_set, DavixError**err);


} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
