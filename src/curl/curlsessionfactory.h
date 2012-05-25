#ifndef CURLSESSIONFACTORY_H
#define CURLSESSIONFACTORY_H

#include <abstractsessionfactory.h>

namespace Davix {

class CURLSessionFactory : public AbstractSessionFactory
{
public:
    CURLSessionFactory();
    virtual ~CURLSessionFactory();

    /**
      Take the ownership on a request object in order to execute a query
      @param typ : type of the request
      @param url : path of the request
      @return Request object
    */
    virtual Request* take_request(RequestType typ, const std::string & url) ;

    /**
        release the ownership on a request object
     */
    virtual void release_request(Request * req);

    /**
      \ref ssl session check
    */
    virtual void set_ssl_ca_check(bool chk);

    virtual void set_authentification_controller(void * userdata, davix_auth_callback call);

private:
    bool _ca_check;
    void * _user_auth_callback_data;
    davix_auth_callback _call;
};

}

#endif // CURLSESSIONFACTORY_H
