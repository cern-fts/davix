#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <errno.h>
#include "request.hpp"

namespace Davix {

class HttpRequest : public Davix::Request
{
public:
    HttpRequest();
    virtual ~HttpRequest();

    /**
      add a personalized header to the header request
      replace an existing one if already exist
      remove one if empty
      @param field : add a header field to the current http request or replace a existing one
      @param value : value of the field to set

    */
    virtual void add_header_field(const std::string & field, const std::string & value) =0;
    /**
     * set the request command to execute ( GET, POST, PUT, PROPFIND )
     */
    virtual void set_requestcustom(const std::string & request_str) =0;

    /**
      disable the certificate authority validity check for the https request
    */
    virtual void disable_ssl_ca_check() = 0;

};


/**
Translate  http code error to errno code
same than in GNU davfs
 */
int httpcode_to_errno(int code);


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
