#ifndef DAVIX_DAVIXHTTPREQUEST_H
#define DAVIX_DAVIXHTTPREQUEST_H

#include <string>
#include <davixrequest.h>
#include <davix_types.h>
#include <davixuri.h>

namespace Davix {

class NGQRequest;
class Context;

class NGQHttpRequest : public NGQRequest
{
public:
    NGQHttpRequest(Context* context, const Uri & my_uri);
    virtual ~NGQHttpRequest(){};

   ///
   ///   add a personalized header to the header request
   ///  replace an existing one if already exist
   ///   remove one if empty
   ///   @param field : add a header field to the current http request or replace a existing one
   ///   @param value : value of the field to set
    virtual void addHeaderField(const std::string & field, const std::string & value)=0;


    /// set the request command to execute ( GET, POST, PUT, PROPFIND )
    /// Default : GET
    inline void setRequestOperation(const std::string & request_str){
        this->ops = request_str;
    }

    inline const Uri & getUri(){
        return _uri;
    }

    inline void setUri(const Uri & uri){
        _uri = uri;
    }

protected:
    std::string ops; // operation
    Uri _uri;

};

} // namespace Davix

#endif // DAVIX_DAVIXHTTPREQUEST_H
