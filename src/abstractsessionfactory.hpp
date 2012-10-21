#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

#include "global_def.hpp"
#include "request.hpp"
#include <davixrequestparams.hpp>

namespace Davix{


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
    virtual Request* create_request(const std::string & url, DavixError** err) =0;

    /**
        delete the request object
     */
    virtual void delete_request(Request * ref)=0;  // throw nothing


};

}

#endif // ABSTRACTSESSIONFACTORY_H
