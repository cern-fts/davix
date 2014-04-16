#ifndef HTTPIOCHAIN_HPP
#define HTTPIOCHAIN_HPP

#include <davix_internal.hpp>

namespace Davix{

class HttpIOChain;


#define CHAIN_FORWARD(X) \
        do{ \
        if(next.get() != NULL){ \
            return next->X; \
        } \
        throw DavixException(davix_scope_io_buff(), StatusCode::OperationNonSupported, "I/O operation not supported"); \
    }while(0)

//
class HttpIOChain : NonCopyable{
public:
    HttpIOChain(Context & c);
    virtual ~HttpIOChain();

    HttpIOChain* add(HttpIOChain* elem);

    void configure(const Uri & uri, const RequestParams * params);

    /*
     *   Meta data opts
     *
     **/
    // calculate hecksum
    virtual void checksum(std::string & checksm, const std::string & chk_algo);

    // calc replica
    virtual void getReplicas(std::vector<DavFile> & vec);

    // delete resource
    virtual void deleteResource();

    // make collection
    virtual void makeCollection();

    // get statInfo
    virtual void statInfo(StatInfo & st_info);


    /*
     *
     *     I/O Layer
     *
     **/
    virtual dav_ssize_t readFull(std::vector<char> & buffer);

    // overloaded version for string content
    virtual dav_ssize_t readFull(std::string & buffer);

    // read to fd
    virtual dav_ssize_t readToFd(int fd, dav_size_t size);

    virtual dav_ssize_t readPartialBufferVec(const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // reset file position and status
    virtual void resetIO();

    // position independant read operation,
    // similar to pread except that does not need open() before
    virtual dav_ssize_t read(void* buf, dav_size_t count, dav_off_t offset);

    // sequential read of a file from begining to the end
    virtual dav_ssize_t read(void* buf, dav_size_t count);


    // position independant write operation,
    // similar to pwrite do not need open() before
    virtual dav_ssize_t writeFromFd(int fd, dav_size_t size);


protected:
    boost::scoped_ptr<HttpIOChain> next;
    Context& _context;
    Uri const* _uri;
    RequestParams const* _params;

};


}

#endif // HTTPIOCHAIN_HPP
