#ifndef DAVIX_COMPOSITION_H
#define DAVIX_COMPOSITION_H

#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <dirent.h>



#include <global_def.h>
#include <davix.h>
#include <httprequest.h>

#include <libdavix_object.h>
#include <abstractsessionfactory.h>



namespace Davix {



class Composition: public Object
{
public:
    Composition();
    virtual ~Composition(){}


    /**
      @brief stat call
      classical POSIX stat call
      @param str: string url
      @param stat : stat struct to fill
     **/
    virtual void stat(const std::string & str, struct stat *)=0;

    /**
      @brief execute an opendir function with Webdav
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    virtual DAVIX_DIR* opendir(const std::string & url)=0;

    /**
      @brief execute a readdir function with Webdav
      @param DAVIX_DIR
    */
    virtual struct dirent* readdir(DAVIX_DIR* )=0;

    /**
      @brief close an existing file handle
    */
    virtual void closedir(DAVIX_DIR* )=0;

    /**
      @brief execute an opendirpp function with Webdav
           opendirpp/readdirpp/closedirpp function read a directory content with stat information for each directory entry
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    virtual DAVIX_DIR* opendirpp(const std::string & url)=0;



    /**
      @brief execute a readdirpp function with Webdav
           opendirpp and readdirpp function read a directory content with stat information for each directory entry
      @param DAVIX_DIR
      @param stat struct to fill
    */
    virtual struct dirent* readdirpp(DAVIX_DIR*, struct stat * st )=0;

    /**
      @brief close an existing file handle
    */
    virtual void closedirpp(DAVIX_DIR* )=0;

    /**
      @brief same behavior than the POSIX mkdir, use MKCOL in background
      @warning dependening of the server, implementation, mode_t parameter can be ignored

    */
    virtual void mkdir(const std::string & url, mode_t right)=0;

    /**
      get the current registered session factory
    */
    virtual AbstractSessionFactory* getSessionFactory()=0;


    /**
      configure the default buffer size for internal transfer
    */
    virtual void set_buffer_size(const size_t value)=0;

};

} // namespace Davix

#endif // DAVIX_COMPOSITION_H
