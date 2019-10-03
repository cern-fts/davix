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

#pragma once
#ifndef DAVIX_IOBUFFMAP_HPP
#define DAVIX_IOBUFFMAP_HPP

#include <davix_internal.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/httpiochain.hpp>



namespace Davix {

//
// Internal POSIX like to HTTP RW operation mapper
// provides facilities for caching
//



/// RW operation mapped to pure HTTP ops
class HttpIO : public HttpIOChain{
public:
    HttpIO();
    virtual ~HttpIO();


    // read to dynamically allocated buffer
    virtual dav_ssize_t readFull(IOChainContext & iocontext, std::vector<char> & buffer);


    // position independant read operation,
    // similar to pread except that does not need open() before
    virtual dav_ssize_t pread(IOChainContext & iocontext, void* buf, dav_size_t count, dav_off_t offset);

    // read to fd
    virtual dav_ssize_t readToFd(IOChainContext & iocontext, int fd, dav_size_t size);

    virtual dav_ssize_t writeFromProvider(IOChainContext & iocontext, ContentProvider &provider);

private:


    HttpIO(const HttpIO & );
    HttpIO & operator=(const HttpIO & );


};


struct IOBufferLocalFile;

///
/// RW operation with buffering support and POSIX like interface
class HttpIOBuffer : public HttpIOChain{
public:
    HttpIOBuffer();
    virtual ~HttpIOBuffer();

    // open the file associated with the davix IOBuffMap
    // do a simple check if the file exist and try to anticipate the next ops
    virtual bool open(IOChainContext & iocontext, int flags);

    //
    virtual dav_ssize_t read(IOChainContext & iocontext, void* buf, dav_size_t count);


    // give information on the future operation for prefecting
    virtual void prefetchInfo(IOChainContext & iocontext, off_t offset, dav_size_t size_read, advise_t adv);

    //
    virtual dav_ssize_t write(IOChainContext & iocontext, const void* buf, dav_size_t count);

    //
    virtual dav_off_t lseek(IOChainContext & iocontext, dav_off_t offset, int flags);

    //
    virtual void resetIO(IOChainContext & iocontext);


    void commitLocal(IOChainContext & iocontext);

protected:

    dav_size_t _file_size;
    bool _file_exist;
    dav_off_t _pos;
    bool _opened;
    advise_t _last_advise;

    // locker
    std::recursive_mutex _rwlock;
    // write cache
    std::unique_ptr<IOBufferLocalFile> _local;

    dav_off_t _read_pos; //curent read file offset
    bool _read_endfile;
    HttpRequest * _read_req;

private:

    inline bool isAdviseFullRead(){
        return (_last_advise == AdviseAuto || _last_advise == AdviseSequential);
    }

    dav_ssize_t readInternal(IOChainContext & iocontext, void *buffer, dav_size_t size_read);

    HttpIOBuffer(const HttpIOBuffer & );
    HttpIOBuffer & operator=(const HttpIOBuffer & );
};

// create a single-ton filestream for cache from
int get_valid_cache_file(FILE** stream, DavixError** err);


dav_ssize_t read_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read, DavixError**err);


} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
