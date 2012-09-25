#ifndef DAVIX_POSIXGATE_H
#define DAVIX_POSIXGATE_H

#include <string>
#include <sys/stat.h>

#include <davix_types.h>
#include <davixgate.h>


namespace Davix {

class Context;
class Gate;

class PosixGate : public Gate
{
public:
    virtual ~PosixGate(){}

    ///  @brief POSIX-like stat call
    ///  retrive minimaliste stat information about the URI
    ///  @param str: string url
    ///  @param stat : stat struct to fill
    ///
    void stat(const std::string & uri, struct stat *);

    /**
      @brief execute an opendir function with Webdav
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    DAVIX_DIR* opendir(const std::string & url);

    /**
      @brief execute a readdir function with Webdav
      @param DAVIX_DIR
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
    DAVIX_DIR* opendirpp(const std::string & url);



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
    void mkdir(const std::string & url, mode_t right);
protected:
    PosixGate(Context* context);
    friend class Context;

};

} // namespace Davix

#endif // DAVIX_POSIXGATE_H
