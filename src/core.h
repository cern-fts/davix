#ifndef DAVIX_CORE_H
#define DAVIX_CORE_H

#include <utility>
#include <iostream>
#include <exception>

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
      implementation of getSessionFactory
    */
    virtual AbstractSessionFactory* getSessionFactory();


     virtual void set_buffer_size(const size_t value);

protected:
    std::auto_ptr<AbstractSessionFactory>  _fsess;
    size_t _s_buff;
    unsigned long _timeout;
};

} // namespace Davix

#endif // DAVIX_CORE_H
