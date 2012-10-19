#ifndef DAVIX_DAVPOSIX_HPP
#define DAVIX_DAVPOSIX_HPP


#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <davixcontext.hpp>
#include <davixrequestparams.hpp>
#include <status/davixstatusrequest.hpp>


/**
  @file davposix.hpp
  @author Devresse Adrien

  @brief C++ POSIX like API of davix
*/

namespace Davix {

class DavPosixInternal;

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
    void stat(const RequestParams* params, const std::string & str, struct stat *);

    /**
      @brief execute an opendir function with Webdav
      behavior similar to the POSIX opendir function

      @param params : request options, can be NULL
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    DAVIX_DIR* opendir(const RequestParams* params, const std::string & url);

    /**
      @brief execute a readdir function with Webdav
      behavior similar to the POSIX readdir function
      @param FileObject
    */
    struct dirent* readdir(DAVIX_DIR* );
    /**
      @brief close an existing file handle
    */
    void closedir(DAVIX_DIR* );

    /**
      @brief execute an opendirpp function with Webdav
           opendirpp/readdirpp/closedirpp function read a directory content with a struct stat associated to  each directory entry

      @param params : request options, can be NULL
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    DAVIX_DIR* opendirpp(const RequestParams* params, const std::string & url);



    /**
      @brief execute a readdirpp function with Webdav
           opendirpp and readdirpp function read a directory content with stat information for each directory entry

      @param params : request options, can be NULL
      @param DAVIX_DIR
      @param stat struct to fill
    */
    struct dirent* readdirpp(DAVIX_DIR*, struct stat * st );

    /**
      @brief close an existing file handle
    */
    void closedirpp(DAVIX_DIR* );

    /**
      @brief execute a mkdir function with Webdav
      behavior similar to the POSIX mkdir function
      @warning dependening of the server, implementation, mode_t parameter can be ignored

      @param params : request options, can be NULL
      @param url: url of the directory to create
      @param right : default mode of the directory ( ignored for now )

    */
    void mkdir(const RequestParams * _params, const std::string & url, mode_t right);


    /**
      @brief open a file for read/write operation in a POSIX-like approach
      behavior similar to the POSIX open function
      This operation is supported on plain HTTP servers.

      @param params : request options, can be NULL
      @param url : url of the HTTP file to open
      @param flags : open flags, similar to the POSIX function open
      @return Davix file descriptor in case of success, or NULL if an error occures.
     */
    DAVIX_FD* open(const RequestParams * _params, const std::string & url, int flags);


    /**
      @brief read a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX read function
      @param fd : davix file descriptor
      @param buf : buffer to fill
      @param count : maximum number of bytes to read
      @return the size of data or a negative value if an error occured
     */
    ssize_t read(DAVIX_FD* fd, void* buf, size_t count);

    /**
      @brief write a file in a POSIX-like approach with HTTP(S)
      behavior similar to the POSIX write function
      @param fd : davix file descriptor
      @param buf : buffer with the write content
      @param count : number of bytes to write
      @return the size of the written data or a negative value if an error occured
     */
    ssize_t write(DAVIX_FD* fd, const void* buf, size_t count);


    /**
      @brief move the cursor a davix file with HTTP(S)
      behavior similar to the POSIX lseek function
      @param fd : davix file descriptor
      @param offset : offset in byte inside the file
      @param flags : lseek flags, similar to the lseek function
      @return the offset position or a negative value if an error occures
     */
    off_t lseek(DAVIX_FD* fd, off_t offset, int flags);

    /**
      @brief close a existing file descriptor

      Note : all file descriptors MUST be closed before the destruction of the parent davix context

      @param fd : davix file descriptor
      @param count : number of bytes to write
      @return 0 if success, negative value if error
     */
    int close(DAVIX_FD* fd);

protected:
    DAVIX_DIR* internal_opendirpp(const RequestParams* params,  const char * scope, const std::string & body, const std::string & url  );

    Context* context;
    long _timeout;
    long _s_buff;


private:
    DavPosixInternal* d_ptr;

};

} // namespace Davix

#endif // DAVIX_DAVPOSIX_HPP
