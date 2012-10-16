#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <errno.h>
#include <davixrequestparams.hpp>
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
    virtual void addHeaderField(const std::string & field, const std::string & value) =0;
    ///
    /// set the request command to execute ( GET, POST, PUT, PROPFIND )
    /// DEFAULT : GET
    virtual void setRequestMethod(const std::string & request_str) =0;

    virtual void set_parameters(const RequestParams &p ){
        params = p;
    }

protected:
    RequestParams params;
};


/**
Translate  http code error to errno code
 */
int httpcode_to_errno(int code);


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
