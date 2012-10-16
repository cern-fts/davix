#ifndef DAVIX_DAVPOSIX_HPP
#define DAVIX_DAVPOSIX_HPP


#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <davixcontext.hpp>
#include <davixrequestparams.hpp>


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
      classical POSIX stat call
      @param str: string url
      @param stat : stat struct to fill
     **/
    void stat(const RequestParams* params, const std::string & str, struct stat *);

    /**
      @brief execute an opendir function with Webdav
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    DAVIX_DIR* opendir(const RequestParams* params, const std::string & url);

    /**
      @brief execute a readdir function with Webdav
      @param FileObject
    */
    struct dirent* readdir(DAVIX_DIR* );
    /**
      @brief close an existing file handle
    */
    void closedir(DAVIX_DIR* );

    /**
      @brief execute an opendirpp function with Webdav
           opendirpp/readdirpp/closedirpp function read a directory content with stat information for each directory entry
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    DAVIX_DIR* opendirpp(const RequestParams* params, const std::string & url);



    /**
      @brief execute a readdirpp function with Webdav
           opendirpp and readdirpp function read a directory content with stat information for each directory entry
      @param DAVIX_DIR
      @param stat struct to fill
    */
    struct dirent* readdirpp(DAVIX_DIR*, struct stat * st );

    /**
      @brief close an existing file handle
    */
    void closedirpp(DAVIX_DIR* );

    /**
      @brief same behavior than the POSIX mkdir, use MKCOL in background
      @warning dependening of the server, implementation, mode_t parameter can be ignored

    */
    void mkdir(const RequestParams * _params, const std::string & url, mode_t right);
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
