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


//
class HttpIOChain : NonCopyable{
public:
    struct IOChainParams{
        IOChainParams(Context & c, const Uri & u, const RequestParams * p): _context(c), _uri(u), _reqparams(p) {}
        Context& _context;
        Uri const & _uri;
        RequestParams const * _reqparams;
    };

    HttpIOChain();
    virtual ~HttpIOChain();

    HttpIOChain* add(HttpIOChain* elem);

    void configure(Context & c, const Uri & uri, const RequestParams * params);

    /*
     *   Meta data opts
     *
     **/
    // calculate hecksum
    virtual void checksum(std::string & checksm, const std::string & chk_algo);

    // calc replica
    virtual std::vector<DavFile> & getReplicas(std::vector<DavFile> & vec);

    // delete resource
    virtual void deleteResource();

    // make collection
    virtual void makeCollection();

    // get statInfo
    virtual StatInfo & statInfo(StatInfo & st_info);


    /*
     *
     *     I/O Layer
     *
     **/

    virtual bool open(int flags);

    virtual void prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv);

    virtual dav_ssize_t readFull(std::vector<char> & buffer);

    // overloaded version for string content
    virtual dav_ssize_t readFull(std::string & buffer);

    // read to fd
    virtual dav_ssize_t readToFd(int fd, dav_size_t size);

    virtual dav_ssize_t preadVec(const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec);

    // reset file position and status
    virtual void resetIO();

    // position independant read operation,
    // similar to pread except that does not need open() before
    virtual dav_ssize_t pread(void* buf, dav_size_t count, dav_off_t offset);

    // sequential read of a file from begining to the end
    virtual dav_ssize_t read(void* buf, dav_size_t count);

    // lseek prototype
    virtual dav_off_t lseek(dav_off_t offset, int flags);


    // position independant write operation,
    // similar to pwrite do not need open() before
    virtual dav_ssize_t writeFromFd(int fd, dav_size_t size);


    // sequential write
    virtual dav_ssize_t write(const void* buf, dav_size_t count);


protected:
    boost::scoped_ptr<HttpIOChain> _next;
    boost::scoped_ptr<IOChainParams> _params;
    HttpIOChain* _start;

    inline IOChainParams & getParams(){
        return *(_start->_params.get());
    }
};


}

#endif // HTTPIOCHAIN_HPP
