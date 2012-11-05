#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

#include "global_def.hpp"
#include "httprequest.hpp"
#include <params/davixrequestparams.hpp>

namespace Davix{

class HttpRequest;

class AbstractSessionFactory
{
public:

    AbstractSessionFactory(){}

    virtual ~AbstractSessionFactory(){}

    /**
      Take the ownership on a request object in order to execute a query
      @param typ : type of the request
      @param url : path of the request
      @return Request object
    */
    virtual HttpRequest* create_request(const std::string & url, DavixError** err) =0;



};

}

#endif // ABSTRACTSESSIONFACTORY_H
