#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

#include "global_def.hpp"
#include "request.hpp"
#include "libdavix_object.hpp"
#include "requestparams.hpp"

namespace Davix{


class AbstractSessionFactory : public Davix::Object
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
    virtual Request* create_request(const std::string & url) =0;

    /**
        delete the request object
     */
    virtual void delete_request(Request * ref)=0;  // throw nothing

    /**
      define a new default set of parameters for all the request issued from this factory
      This set of parameters can be overriten at the request level
    */
    virtual void set_parameters(const RequestParams &p ){
        params = p;
    }

protected:
    RequestParams params;
};

}

#endif // ABSTRACTSESSIONFACTORY_H
