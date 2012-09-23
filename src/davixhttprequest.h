#ifndef DAVIX_DAVIXHTTPREQUEST_H
#define DAVIX_DAVIXHTTPREQUEST_H

#include <string>
#include <davixrequest.h>
#include <davix_types.h>

namespace Davix {

class NGQRequest;
class Context;

class NGQHttpRequest : public NGQRequest
{
public:
    NGQHttpRequest(Context* context);
    virtual ~NGQHttpRequest(){};

   ///
   ///   add a personalized header to the header request
   ///  replace an existing one if already exist
   ///   remove one if empty
   ///   @param field : add a header field to the current http request or replace a existing one
   ///   @param value : value of the field to set
    virtual void addHeaderField(const std::string & field, const std::string & value)=0;


    /// set the request command to execute ( GET, POST, PUT, PROPFIND )
    /// DEFAULT : GET
    inline void setRequestOperation(const std::string & request_str){
        this->ops = request_str;
    }


   ///   disable the certificate authority validity check for the https request
    inline void set_ssl_ca_check(bool chk){
           this->ssl_check = chk;
    }


    ///  authentification callback for right management
    inline void set_authentification_controller(void * userdata, davix_auth_callback call){
        this->userdata = userdata;
        this->call = call;
    }

    ///  define the connexion timeout in seconds
    void set_connexion_timeout(const struct timespec * conn_timeout);

    ///  define the operation timeout in seconds
    void set_operation_timeout(const struct timespec * ops_timeout);

protected:
    std::string ops; // operation
    struct timespec conn_timeout; // timeouts
    struct timespec ops_timeout;
    void * userdata;  // authentification datas
    davix_auth_callback call;
    bool ssl_check;

};

} // namespace Davix

#endif // DAVIX_DAVIXHTTPREQUEST_H
