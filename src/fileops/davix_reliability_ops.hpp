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
