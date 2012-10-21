#include "davixuri.hpp"
#include <ne_uri.h>
#include <cstring>

namespace Davix {

static std::string void_str;

struct UriPrivate{
    UriPrivate(){

    }

    ne_uri my_uri;
    std::string proto;
    std::string path;
    std::string host;
};

Uri::Uri(){
    _init();
}

void Uri::_init(){
    d_ptr = new UriPrivate();
    code = StatusCode::UriParsingError;
}

Uri::Uri(const std::string & uri)
{
    this->uri_string = uri;
    _init();
   if(ne_uri_parse(uri_string.c_str(), &(d_ptr->my_uri)) == 0){

       // fix a neon parser bug when port != number
       if(d_ptr->my_uri.port == 0 && strcasecmp(d_ptr->my_uri.scheme, "http") ==0)
           d_ptr->my_uri.port = 80;

       if(d_ptr->my_uri.port == 0 && strcasecmp(d_ptr->my_uri.scheme, "https") ==0)
           d_ptr->my_uri.port = 443;

       if(d_ptr->my_uri.port == 0)
           return;

       code = StatusCode::OK;
       d_ptr->proto = d_ptr->my_uri.scheme;
       d_ptr->path = d_ptr->my_uri.path;
       d_ptr->host = d_ptr->my_uri.host;
   }
}

Uri::~Uri(){
    ne_uri_free(&(d_ptr->my_uri));
    delete d_ptr;
}

int Uri::getPort() const{
    if(code != StatusCode::OK)
        return -1;
    return d_ptr->my_uri.port;
}

const std::string & Uri::getHost() const{
    if(code != StatusCode::OK)
        return void_str;
    return d_ptr->host;
}

const std::string & Uri::getString() const{
    return uri_string;
}

const std::string & Uri::getProtocol() const {
    if(code != StatusCode::OK)
        return void_str;
    return d_ptr->proto;
}

const std::string & Uri::getPath() const {
    if(code != StatusCode::OK)
        return void_str;
    return d_ptr->path;
}

StatusCode::Code Uri::getStatus() const{
    return code;
}

} // namespace Davix
