#ifndef HTTPIOCHAIN_HPP
#define HTTPIOCHAIN_HPP

#include <davix_internal.hpp>

namespace Davix{

class HttpIOChain;


#define CHAIN_FORWARD(X) \
        do{ \
        if(_next.get() != NULL){ \
            return _next->X; \
        } \
        throw DavixException(davix_scope_io_buff(), StatusCode::OperationNonSupported, "I/O operation not supported"); \
    }while(0)



// parameter handler for any IO Chain operation
struct IOChainContext{
    IOChainContext(Context & c, const Uri & u, const RequestParams * p): _context(c), _uri(u), _reqparams(p) {}
    Context& _context;
    Uri const & _uri;
    RequestParams const * _reqparams;
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

    // get statInfo
    virtual StatInfo & statInfo(IOChainContext & iocontext, StatInfo & st_info);

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


    // position independant write operation,
    // similar to pwrite do not need open() before
    virtual dav_ssize_t writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size);


    // sequential write
    virtual dav_ssize_t write(IOChainContext & iocontext, const void* buf, dav_size_t count);


protected:
    boost::scoped_ptr<HttpIOChain> _next;
    HttpIOChain* _start;


};


}

#endif // HTTPIOCHAIN_HPP
