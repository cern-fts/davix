#ifndef DAVIX_DAVPOSIX_HPP
#define DAVIX_DAVPOSIX_HPP



#include <davix_file_types.hpp>
#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <status/davixstatusrequest.hpp>


/**
  @file davposix.hpp
  @author Devresse Adrien

  @brief POSIX-like API of davix 
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp for the C++ API or davix.h for the C API should be included."
#endif



namespace Davix {


class DavPosixInternal;


///
/// @brief POSIX-like API of Davix
///
/// DavPosix offers a POSIX-like API for HTTP/WebDav file operations
///
/// POSIX API can be used for convenience when porting application
/// to a POSIX like interface.
///
/// DavPosix and all associated method are Thread safe.
class DAVIX_EXPORT DavPosix
{
public:
    ///
    /// \brief default constructor
    /// \param handle
    ///
    DavPosix(Context* handle);
    virtual ~DavPosix();


    /**
      @brief POSIX-like stat() call
      
      
      behavior similar to the POSIX stat function, see man 3 stat
      Supported by Webdav, Http and S3
      Depending of the protocol, some struct stat field can be ignored.
      
      
      @param params request options, can be NULL
      @param str string url
      @param st stat struct to fill
      @param err Davix error report system
      @return 0 if success, negative value if error
     **/
    int stat(const RequestParams* params, const std::string & str, struct stat * st, DavixError** err);

    /**
      @brief open a directory for listing
      
      behavior similar to the POSIX opendir function
      Supported by Webdav

      @param params request options, can be NULL
      @param url url of the directory to list
      @param err Davix error report system      
      @return DAVIX_DIR davix readdir handle, NULL if error
    */
    DAVIX_DIR* opendir(const RequestParams* params, const std::string & url, DavixError** err);

    /**
      @brief read an entry directory
           
      behavior similar to the POSIX readdir function

      @param dir directory handle
      @param err Davix Error report
      @return dirent struct if success, or NULL if error
    */
    struct dirent* readdir(DAVIX_DIR* dir, DavixError** err);
    /**
       @brief close a directory handle
       
       @param  d directory handle to close
       @param err Davix error report system
       @return 0 if success else a negative value and err is set.
    */
    int closedir(DAVIX_DIR* d, DavixError** err);

    /**
      @brief open a directory for listing with per entry meta-data informations
      
      Similar to \ref Davix::DavPosix::opendir but provide stat() informations for each entry
      Supported by Webdav

      @param params request options, can be NULL
      @param url url of the directory to list      
      @param err Davix error report system
      @return DAVIX_DIR davix readdir handle or NULL if error, in this case err is set.
    */
    DAVIX_DIR* opendirpp(const RequestParams* params, const std::string & url, DavixError** err);



    /**
      @brief execute an readdirpp function
      
      Similar to \ref Davix::DavPosix::readdir but provide stat() informations for each entry
      Supported by Webdav

      @param dir directory handle
      @param st struct to fill
      @param err Davix Error report
      @return dirent struct if success, or NULL if error
    */
    struct dirent* readdirpp(DAVIX_DIR* dir, struct stat * st, DavixError** err );

    /**
       @brief close a directory handle
       
       @param  d directory handle to close
       @param err Davix error report system
       @return 0 if success else a negative value and err is set.
    */
    int closedirpp(DAVIX_DIR* d, DavixError** err );

    /**
      @brief execute a mkdir function with Webdav
      behavior similar to the POSIX mkdir function
      @warning dependening of the server, implementation, mode_t parameter can be ignored

      @param params request options, can be NULL
      @param url url of the directory to create
      @param right default mode of the directory ( ignored for now )
      @param err Davix error report system
      @return 0 if success else a negative value and err is set.

    */
    int mkdir(const RequestParams* params, const std::string& url, mode_t right, DavixError** err);

    /**
      @brief execute a remove file operation
      behavior similar to the POSIX unlink function

      @param params request options, can be NULL
      @param url file to delete
      @param err Davix error report system
      @return 0 if success else a negative value and err is set.
    */
    int unlink(const RequestParams* params, const std::string& url, DavixError** err);

    /**
      @brief execute a remove directory operation
      behavior similar to the POSIX rmdir function

      @param params request options, can be NULL
      @param url directory to delete
      @param err Davix error report system
      @return 0 if success else a negative value and err is set.
    */
    int rmdir(const RequestParams* params, const std::string& url, DavixError** err);


    /**
      @brief open a file for read/write operation in a POSIX-like approach
      behavior similar to the POSIX open function
      This operation is supported on plain HTTP servers.

      @param params request options, can be NULL
      @param url url of the HTTP file to open
      @param flags open flags, similar to the POSIX function open
      @param err Davix Error report
      @return Davix file descriptor in case of success, or NULL if an error occures.
     */
    DAVIX_FD* open(const RequestParams* params, const std::string & url, int flags, DavixError** err);


    /**
      @brief read a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX read function
      @param fd davix file descriptor
      @param buffer buffer to fill
      @param count maximum number of bytes to read
      @param err Davix Error report
      @return the size of data or a negative value if an error occured
     */
    ssize_t read(DAVIX_FD* fd, void* buffer, size_t count, DavixError** err);


    /**
      @brief do a partial read of a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX pread function
      @param fd davix file descriptor
      @param buffer buffer to fill
      @param count maximum number of bytes to read
      @param offset  offset to use
      @param err Davix Error report
      @return the size of data or a negative value if an error occured
     */
    ssize_t pread(DAVIX_FD* fd, void* buffer, size_t count, off_t offset, DavixError** err);

    /**
      @brief do a partial write of a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX pwrite function
      @param fd davix file descriptor
      @param buffer buffer to fill
      @param count maximum number of bytes to write
      @param offset  offset to use
      @param err Davix Error report
      @return the size of data written or a negative value if an error occured
     */
    ssize_t pwrite(DAVIX_FD* fd, const void* buffer, size_t count, off_t offset, DavixError** err);

    /**
      @brief pread_vec a file in a POSIX-like approach with HTTP(S)
            Vector read operation
            Allow to do several read operations in one single request
      @param fd davix file descriptor
      @param input_vec input vectors, parameters
      @param output_vec output vectors, results
      @param count_vec number of vector struct
      @param err Davix Error report
      @return total number of bytes read, or -1 if error occures
     */
    dav_ssize_t preadVec(DAVIX_FD* fd, const DavIOVecInput * input_vec,
                          DavIOVecOuput * output_vec,
                          dav_size_t count_vec, DavixError** err);

    /**
      @brief write a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX write function
      @param fd davix file descriptor
      @param buf buffer with the write content
      @param count number of bytes to write
      @param err Davix Error report
      @return the size of the written data or a negative value if an error occured

     */
    ssize_t write(DAVIX_FD* fd, const void* buf, size_t count, DavixError** err);


    /**
      @brief move the cursor a davix file with HTTP(S)
      behavior similar to the POSIX lseek function
      @param fd davix file descriptor
      @param offset offset in byte inside the file
      @param flags lseek flags, similar to the lseek function
      @param err Davix Error report
      @return the offset position or a negative value if an error occures
     */
    off_t lseek(DAVIX_FD* fd, off_t offset, int flags, DavixError** err);

    /**
      @brief close a existing file descriptor

      Note : all file descriptors MUST be closed before the destruction of the parent davix context

      @param fd davix file descriptor
      @param err Davix Error report
      @return 0 if success, negative value if error
     */
    int close(DAVIX_FD* fd, DavixError** err);

    /**
      @brief give advise about next file operation

      similar to posix_fadvise, allow I/O optimizations
      non-blocking asynchronous function

      @param fd Davix file descriptor
      @param offset offset of the next chunk to read
      @param len size of the next chunk to read
      @param advise type of pattern for I/O : sequential, random
    */
    void fadvise(DAVIX_FD* fd, dav_off_t offset, dav_size_t len, advise_t advice);

private:
    DAVIX_DIR* internal_opendirpp(const RequestParams* params,  const char * scope, const std::string & body, const std::string & url, DavixError** err);

    Context* context;
    long _timeout;
    long _s_buff;


    DavPosix(const DavPosix &);
    DavPosix & operator=(DavPosix &);

    DavPosixInternal* d_ptr;

};


} // namespace Davix

#endif // DAVIX_DAVPOSIX_HPP
