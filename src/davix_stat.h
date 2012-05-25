#ifndef DAVIX_STAT_H
#define DAVIX_STAT_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "composition.h"
#include "abstractsessionfactory.h"
#include "httprequest.h"

namespace Davix{

class Composition;

class Stat : public Object
{
public:
    Stat(Composition* obj, AbstractSessionFactory* fsess);

    /**
      @brief stat call
      classical POSIX stat call
      @param str: string url
      @param stat : stat struct to fill
     **/
    void stat(const std::string & str, struct stat *);
protected:
    Composition* _core;
    AbstractSessionFactory* _fsess;


};

/**
  retrieve a webdav propfind stat request to the given url
    @param req : http request where to executethe query
    @return vector of characters of the query content
    @throw GLib::Error
  */
const std::vector<char> & req_webdav_propfind(HttpRequest* req); // throw Glib::Error

}

#endif // DAVIX_STAT_H
