#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

 
#include "httprequest.hpp"
#include <params/davixrequestparams.hpp>
#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

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
    virtual HttpRequest* create_request(const Uri & uri, DavixError** err)=0;



};

}

#endif // ABSTRACTSESSIONFACTORY_H
