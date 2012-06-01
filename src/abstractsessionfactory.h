#ifndef ABSTRACTSESSIONFACTORY_H
#define ABSTRACTSESSIONFACTORY_H

#include "global_def.h"
#include "request.h"
#include "libdavix_object.h"

namespace Davix{

class AbstractSessionFactory : public Davix::Object
{
public:
    /**
      Take the ownership on a request object in order to execute a query
      @param typ : type of the request
      @param url : path of the request
      @return Request object
    */
    virtual Request* take_request(RequestType typ, const std::string & url) =0;

    /**
        release the ownership on a request object
     */
    virtual void release_request(Request * ref)=0;

    /**
      disable the certificate authority validity check for the https request
    */
    virtual void set_ssl_ca_check(bool chk) =0;

    /**
      authentification callback for right management
    */
    virtual void set_authentification_controller(void * userdata, davix_auth_callback call)=0;
};

}

#endif // ABSTRACTSESSIONFACTORY_H
