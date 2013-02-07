#ifndef DAVIX_DAVPOSIX_HPP
#define DAVIX_DAVPOSIX_HPP


#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <status/davixstatusrequest.hpp>


/**
  @file davposix.hpp
  @author Devresse Adrien

  @brief POSIX-like API of davix C++
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix {

class DavPosixInternal;

///
/// @brief POSIX-like API of Davix
///
/// DavPosix offers POSIX like API for HTTP/WebDav file operations
/// Thread safe
class DavPosix
{
public:
    DavPosix(Context* handle);
    virtual ~DavPosix();


    /**
      @brief stat call
      behavior similar to the POSIX stat function
      @param params : request options, can be NULL
      @param str: string url
      @param stat : stat struct to fill
     **/
    int stat(const RequestParams* params, const std::string & str, struct stat *, DavixError** err);

    /**
      @brief execute an opendir function with Webdav
      behavior similar to the POSIX opendir function

      @param params : request options, can be NULL
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @return
    */
    DAVIX_DIR* opendir(const RequestParams* params, const std::string & url, DavixError** err);

    /**
      @brief execute a readdir function with Webdav
      behavior similar to the POSIX readdir function

      @param params : request options, can be NULL
      @param dir : directory handle
      @param err: Davix Error report
      @return dirent struct if success, or NULL if error
    */
    struct dirent* readdir(DAVIX_DIR* dir, DavixError** err);
    /**
       close an existing file handle
       @param  d : directory handle to close
       @param err : Davix error report system
       @return 0 if success else a negative value and err is set.
    */
    int closedir(DAVIX_DIR* d, DavixError** err);

    /**
      @brief execute an opendirpp function with Webdav
           opendirpp/readdirpp/closedirpp function read a directory content with a struct stat associated to  each directory entry

      @param params : request options, can be NULL
      @param err : Davix error report system
      @return DAVIX_DIR : davix readdir handle or NULL if error, in this case err is set.
    */
    DAVIX_DIR* opendirpp(const RequestParams* params, const std::string & url, DavixError** err);



    /**
      @brief execute a readdirpp function with Webdav
           opendirpp and readdirpp function read a directory content with stat information for each directory entry

      @param params : request options, can be NULL
      @param dir : directory handle
      @param stat struct to fill
      @param err: Davix Error report
      @return dirent struct if success, or NULL if error
    */
    struct dirent* readdirpp(DAVIX_DIR* dir, struct stat * st, DavixError** err );

    /**
      @brief close an existing file handle
    */
    int closedirpp(DAVIX_DIR*, DavixError** err );

    /**
      @brief execute a mkdir function with Webdav
      behavior similar to the POSIX mkdir function
      @warning dependening of the server, implementation, mode_t parameter can be ignored

      @param params : request options, can be NULL
      @param url: url of the directory to create
      @param right : default mode of the directory ( ignored for now )
      @param err : Davix error report system
      @return 0 if success else a negative value and err is set.

    */
    int mkdir(const RequestParams * _params, const std::string & url, mode_t right, DavixError** err);

    /**
      @brief execute a remove file operation
      behavior similar to the POSIX unlink function

      @param params : request options, can be NULL
      @param url: file to delete
      @param err : Davix error report system
      @return 0 if success else a negative value and err is set.
    */
    int unlink(const RequestParams * _params, const std::string &url, DavixError** err);

    /**
      @brief execute a remove directory operation
      behavior similar to the POSIX rmdir function

      @param params : request options, can be NULL
      @param url: directory to delete
      @param err : Davix error report system
      @return 0 if success else a negative value and err is set.
    */
    int rmdir(const RequestParams * _params, const std::string &url, DavixError** err);


    /**
      @brief open a file for read/write operation in a POSIX-like approach
      behavior similar to the POSIX open function
      This operation is supported on plain HTTP servers.

      @param params : request options, can be NULL
      @param url : url of the HTTP file to open
      @param flags : open flags, similar to the POSIX function open
      @param err: Davix Error report
      @return Davix file descriptor in case of success, or NULL if an error occures.
     */
    DAVIX_FD* open(const RequestParams * _params, const std::string & url, int flags, DavixError** err);


    /**
      @brief read a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX read function
      @param fd : davix file descriptor
      @param buf : buffer to fill
      @param count : maximum number of bytes to read
      @param err: Davix Error report
      @return the size of data or a negative value if an error occured
     */
    ssize_t read(DAVIX_FD* fd, void* buf, size_t count, DavixError** err);

    /**
      @brief do a partial read of a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX pread function
      @param fd : davix file descriptor
      @param buf : buffer to fill
      @param count : maximum number of bytes to read
      @param offset : offset to use
      @param err: Davix Error report
      @return the size of data or a negative value if an error occured
     */
    ssize_t pread(DAVIX_FD* fd, void* buf, size_t count, off_t offset, DavixError** err);

    /**
      @brief write a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX write function
      @param fd : davix file descriptor
      @param buf : buffer with the write content
      @param count : number of bytes to write
      @return the size of the written data or a negative value if an error occured
     */
    ssize_t write(DAVIX_FD* fd, const void* buf, size_t count, DavixError** err);


    /**
      @brief move the cursor a davix file with HTTP(S)
      behavior similar to the POSIX lseek function
      @param fd : davix file descriptor
      @param offset : offset in byte inside the file
      @param flags : lseek flags, similar to the lseek function
      @return the offset position or a negative value if an error occures
     */
    off_t lseek(DAVIX_FD* fd, off_t offset, int flags, DavixError** err);

    /**
      @brief close a existing file descriptor

      Note : all file descriptors MUST be closed before the destruction of the parent davix context

      @param fd : davix file descriptor
      @param count : number of bytes to write
      @return 0 if success, negative value if error
     */
    int close(DAVIX_FD* fd, DavixError** err);


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
