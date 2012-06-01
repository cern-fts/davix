#ifndef DAVIX_CORE_H
#define DAVIX_CORE_H

#include <utility>
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

#include <composition.h>
#include <global_def.h>



namespace Davix {

/**
  composition of the differents HTTP operations
 */
class Core : public Composition
{
public:
    Core(AbstractSessionFactory * fsess);

    static Glib::RefPtr<Core> create(AbstractSessionFactory* fsess);


    /**
      @brief stat call
      classical POSIX stat call
      @param str: string url
      @param stat : stat struct to fill
     **/
    virtual void stat(const std::string & str, struct stat *);

    /**
      @brief execute an opendir function with Webdav
      @param url : url of the directory to list
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    virtual DAVIX_DIR* opendir(const std::string & url);

    /**
      @brief execute a readdir function with Webdav
      @param FileObject
    */
    virtual struct dirent* readdir(DAVIX_DIR* );
    /**
      @brief close an existing file handle
    */
    virtual void closedir(DAVIX_DIR* );

    /**
      @brief execute an opendirpp function with Webdav
           opendirpp/readdirpp/closedirpp function read a directory content with stat information for each directory entry
      @return DAVIX_DIR : davix readdir handle
      @throw : Glib::Error, with a errno code value
      @return
    */
    virtual DAVIX_DIR* opendirpp(const std::string & url);



    /**
      @brief execute a readdirpp function with Webdav
           opendirpp and readdirpp function read a directory content with stat information for each directory entry
      @param DAVIX_DIR
      @param stat struct to fill
    */
    virtual struct dirent* readdirpp(DAVIX_DIR*, struct stat * st );

    /**
      @brief close an existing file handle
    */
    virtual void closedirpp(DAVIX_DIR* );

    /**
      @brief same behavior than the POSIX mkdir, use MKCOL in background
      @warning dependening of the server, implementation, mode_t parameter can be ignored

    */
    virtual void mkdir(const std::string & url, mode_t right);

    /**
      implementation of getSessionFactory
    */
    virtual AbstractSessionFactory* getSessionFactory();


     virtual void set_buffer_size(const size_t value);

protected:

    DAVIX_DIR* internal_opendirpp(const char * scope, const std::string & body, const std::string & url  );

    std::auto_ptr<AbstractSessionFactory>  _fsess;
    size_t _s_buff;
    unsigned long _timeout;
};

} // namespace Davix

#endif // DAVIX_CORE_H
