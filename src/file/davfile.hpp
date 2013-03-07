#ifndef DAVFILE_HPP
#define DAVFILE_HPP

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <posix/davposix.hpp>
#include <davix_types.h>



#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix{

struct DavFileInternal;


typedef std::vector<Uri> ReplicaVec;

class DavFile
{
public:
    DavFile(Context & c, const Uri & u);
    virtual ~DavFile();

    ///
    /// @brief return all replicas associated to this file
    ///
    /// Replicas are found using a corresponding meta-link file or Webdav extensions if supported
    ///
    /// @param params: Davix Request parameters
    /// @param vec : Replica vector
    /// @param err : DavixError error report
    /// @return  the number of replicas if found, -1 if error.
    dav_ssize_t getAllReplicas(const RequestParams* params,
                               ReplicaVec & vec,
                               DavixError** err);

    ///
    ///  @brief Vector read operation
    ///        Allow to do several read several data chunk in one single operation
    ///        Use Http multi-part when supported by the server,
    ///        simulate a vector read operation in the other case
    ///
    ///  @param fd : davix file descriptor
    ///  @param input_vec : input vectors, parameters
    ///  @param output_vec : output vectors, results
    ///  @param count_vec : number of vector struct
    ///  @param err: Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t readPartialBufferVec(const RequestParams* params,
                          const DavIOVecInput * input_vec,
                          DavIOVecOuput * ioutput_vec,
                          const dav_size_t count_vec,
                          DavixError** err);

    ///
    ///  @brief Partial position independant read.
    ///         Use Ranged request when supported by the server,
    ///               simulate a ranged request when not  supported
    ///
    ///  @param buff : buffer
    ///  @param count : maximum read size
    ///  @param offset : start offset  for the read operation
    ///  @param err: Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t readPartial(const RequestParams* params,
                            void* buff,
                            dav_size_t count,
                            dav_off_t offset,
                            DavixError** err);



private:
    DavFileInternal* d_ptr;
    DavFile(const DavFile & f);
    DavFile & operator=(const DavFile & f);

};




} // Davix

#endif // DAVFILE_HPP
