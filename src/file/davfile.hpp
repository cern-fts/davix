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
    DavFile(Context & c, const Uri & url);
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
    ///  @param params: Davix request Parameters
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
    ///
    ///
    ///         Use Ranged request when supported by the server,
    ///         simulate a ranged request when not  supported
    ///
    ///  @param params: Davix request Parameters
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


    ///
    ///  @brief Get Full content of the file and write it to fd
    ///
    ///  @param params: Davix request Parameters
    ///  @param fd : file descriptor for write operation
    ///  @param err: Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            DavixError** err);

    ///
    ///  @brief Get the first 'size_read' bytes of the file and write it to fd
    ///
    ///  @param params: Davix request Parameters
    ///  @param fd : file descriptor for write operation
    ///  @param err: Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getToFd(const RequestParams* params,
                            int fd,
                            dav_size_t size_read,
                            DavixError** err);

    ///
    ///  @brief Get the file content
    ///
    ///  @param params: Davix request Parameters
    ///  @param fd : file descriptor for write operation
    ///  @param err: Davix Error report
    ///  @return total number of bytes read, or -1 if error occures
    ///
    dav_ssize_t getFull(const RequestParams* params,
                            std::vector<char> & buffer,
                            DavixError** err);

    ///
    ///  @brief Create/replace the file with the content
    ///     of the file descriptor fd
    ///
    ///  @param params: Davix request Parameters
    ///  @param fd : file descriptor
    ///  @param err: Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    int putFromFd(const RequestParams* params,
                  int fd,
                  dav_size_t size,
                  DavixError** err);



    ///
    ///  @brief Suppress the current entity.
    ///         able to suppress collection too
    ///
    ///  @param params: Davix request Parameters
    ///  @param err: Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    int deletion(const RequestParams* params,
                 DavixError** err);


    ///
    /// @brief create a collection ( directory or bucket) at the current file url
    ///
    ///  @param params: Davix request Parameters
    ///  @param err: Davix Error report
    ///  @return 0 if success, or -1 if error occures
    ///
    int makeCollection(const RequestParams* params,
                 DavixError** err);



private:
    DavFileInternal* d_ptr;
    DavFile(const DavFile & f);
    DavFile & operator=(const DavFile & f);

};




} // Davix

#endif // DAVFILE_HPP
