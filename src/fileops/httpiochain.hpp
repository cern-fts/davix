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

#ifndef HTTPIOCHAIN_HPP
#define HTTPIOCHAIN_HPP

#include <davix_internal.hpp>

namespace Davix{

class HttpIOChain;
class ContentProvider;

#define CHAIN_FORWARD(X) \
        do{ \
        if(_next.get() != NULL){ \
            return _next->X; \
        } \
        throw DavixException(davix_scope_io_buff(), StatusCode::OperationNonSupported, "I/O operation not supported"); \
    }while(0)


// stores state for readToFd operations - necessary, so as not to write the same
// data again to an fd after a retry / metalink recovery.
struct FdHandler {
    FdHandler() : fd(-1), bytes_written_to_fd(0) { }

    int fd;
    dav_ssize_t bytes_written_to_fd;
};


// parameter handler for any IO Chain operation
struct IOChainContext{
    IOChainContext(Context & c, const Uri & u, const RequestParams * p): _context(c), _uri(u), _reqparams(p), _end_time() {
        if(_reqparams->getOperationTimeout()->tv_sec > 0){
            _end_time = Chrono::Clock(Chrono::Clock::Monolitic).now();
            _end_time += Chrono::Duration(_reqparams->getOperationTimeout()->tv_sec);
        }
    }


    void checkTimeout(){
        if( _end_time.isValid() && _end_time < Chrono::Clock(Chrono::Clock::Monolitic).now()){
            std::ostringstream ss;
            ss << "operation timeout of " << _reqparams->getOperationTimeout()->tv_sec << "s expired";
            throw DavixException(davix_scope_io_buff(), StatusCode::OperationTimeout, ss.str());
        }
    }




    // context parameter
    Context& _context;
    Uri const & _uri;
    RequestParams const * _reqparams;

    // Operation parameter
    Chrono::TimePoint _end_time;

    // Keep track of how many bytes we've written to an fd, so as to avoid
    // writing the same bytes again in an event of retries / metalink recovery
    FdHandler fdHandler;
};

// Davix IO chain
// Complete I/O Stack implementation of Davix
class HttpIOChain : NonCopyable{
public:

    HttpIOChain();
    virtual ~HttpIOChain();

    HttpIOChain* add(HttpIOChain* elem);

    /*
     *   Meta data opts
     *
     **/
    // calculate hecksum
    virtual void checksum(IOChainContext & iocontext, std::string & checksm, const std::string & chk_algo);

    // calc replica
    virtual std::vector<DavFile> & getReplicas(IOChainContext & iocontext, std::vector<DavFile> & vec);

    // delete resource
    virtual void deleteResource(IOChainContext & iocontext);

    // make collection
    virtual void makeCollection(IOChainContext & iocontext);

    // move/rename resource
    virtual void move(IOChainContext & iocontext, const std::string & target_url);

    // get statInfo
    virtual StatInfo & statInfo(IOChainContext & iocontext, StatInfo & st_info);

    // get quota info
    virtual QuotaInfo & quotaInfo(IOChainContext & iocontext, QuotaInfo & info);

    // listing
    // return false if end of directory is reached
    virtual bool nextSubItem(IOChainContext & iocontext, std::string & entry_name, StatInfo & info);


    /*
     *     I/O Layer
     **/
    virtual bool open(IOChainContext & iocontext, int flags);

    virtual void prefetchInfo(IOChainContext & iocontext, off_t offset, dav_size_t size_read, advise_t adv);

    virtual dav_ssize_t readFull(IOChainContext & iocontext, std::vector<char> & buffer);

    // overloaded version for string content
    virtual dav_ssize_t readFull(IOChainContext & iocontext, std::string & buffer);

    // read to fd
    virtual dav_ssize_t readToFd(IOChainContext & iocontext, int fd, dav_size_t size);

    virtual dav_ssize_t preadVec(IOChainContext & iocontext, const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // reset file position and status
    virtual void resetIO(IOChainContext & iocontext);

    // position independant read operation,
    // similar to pread except that does not need open() before
    virtual dav_ssize_t pread(IOChainContext & iocontext, void* buf, dav_size_t count, dav_off_t offset);

    // sequential read of a file from begining to the end
    virtual dav_ssize_t read(IOChainContext & iocontext, void* buf, dav_size_t count);

    // lseek prototype
    virtual dav_off_t lseek(IOChainContext & iocontext, dav_off_t offset, int flags);

    // sequential write
    virtual dav_ssize_t write(IOChainContext & iocontext, const void* buf, dav_size_t count);

    // write provided contents
    virtual dav_ssize_t writeFromProvider(IOChainContext & iocontext, ContentProvider &provider);

protected:
    std::unique_ptr<HttpIOChain> _next;
    HttpIOChain* _start;


};


}

#endif // HTTPIOCHAIN_HPP
