#include "davixuri.hpp"
#include "davixuri.h"
#include <ne_uri.h>
#include <cstring>

namespace Davix {

/**
 * @cond HIDDEN_SYMBOLS
 */

static std::string void_str;

struct UriPrivate{
    UriPrivate() :
        my_uri(),
        code(StatusCode::UriParsingError),
        proto(),
        path(),
        host(),
        query(),
        query_and_path(){

        memset(&my_uri, 0, sizeof(my_uri));
    }

    UriPrivate(const UriPrivate & orig):
        my_uri(),
        code(orig.code),
        proto(orig.proto),
        path(orig.path),
        host(orig.host),
        query(orig.query),
        query_and_path(orig.query_and_path){
        ne_uri_copy(&my_uri, &(orig.my_uri));

    }

    ~UriPrivate() {
        ne_uri_free(&my_uri);
    }

    void parsing(const std::string & uri_string){
        if(ne_uri_parse(uri_string.c_str(), &(my_uri)) == 0){

            if(my_uri.scheme == NULL
               || my_uri.path == NULL
               || my_uri.host ==NULL)
                return;

            // fix a neon parser bug when port != number
            if(my_uri.port == 0 && strcasecmp(my_uri.scheme, "http") ==0)
                my_uri.port = 80;

            if(my_uri.port == 0 && strcasecmp(my_uri.scheme, "https") ==0)
                my_uri.port = 443;

            if(my_uri.port == 0)
                return;

            code = StatusCode::OK;
            proto = my_uri.scheme;
            path = my_uri.path;
            host = my_uri.host;
            if(my_uri.query){
                query = my_uri.query;
                query_and_path= path + "?" + query;
            }else{
                query_and_path = path;
            }

        }
    }

    ne_uri my_uri;
    StatusCode::Code code;
    std::string proto;
    std::string path;
    std::string host;
    std::string query;
    std::string query_and_path;

};

Uri::Uri() :
    uri_string(),
    d_ptr(new UriPrivate()){
}


Uri::Uri(const std::string & uri) :
    uri_string(uri),
    d_ptr(new UriPrivate()){
    d_ptr->parsing(uri);
}

Uri::Uri(const Uri & uri) :
    uri_string(uri.uri_string),
    d_ptr(new UriPrivate(*(uri.d_ptr))){
}

Uri::~Uri(){
    ne_uri_free(&(d_ptr->my_uri));
    delete d_ptr;
}

int Uri::getPort() const{
    if(d_ptr->code != StatusCode::OK)
        return -1;
    return d_ptr->my_uri.port;
}

const std::string & Uri::getHost() const{
    if(d_ptr->code != StatusCode::OK)
        return void_str;
    return d_ptr->host;
}

const std::string & Uri::getString() const{
    return uri_string;
}

const std::string & Uri::getProtocol() const {
    if(d_ptr->code != StatusCode::OK)
        return void_str;
    return d_ptr->proto;
}

const std::string & Uri::getPath() const {
    if(d_ptr->code != StatusCode::OK)
        return void_str;
    return d_ptr->path;
}

const std::string & Uri::getPathAndQuery() const {
    if(d_ptr->code != StatusCode::OK)
        return void_str;
    return d_ptr->query_and_path;
}

const std::string & Uri::getQuery() const{
    if(d_ptr->code != StatusCode::OK)
        return void_str;
    return d_ptr->query;
}

Uri & Uri::operator =(const Uri & orig){
    if (d_ptr) delete d_ptr;
    d_ptr = new UriPrivate(*(orig.d_ptr));
    return *this;
}

StatusCode::Code Uri::getStatus() const{
    return d_ptr->code;
}

bool uriCheckError(const Uri &uri, DavixError **err){
    if(uri.getStatus() == StatusCode::OK)
        return true;
    DavixError::setupError(err, davix_scope_uri_parser(), uri.getStatus(), std::string("Uri syntax Invalid : ").append(uri.getString()));
    return false;
}

} // namespace Davix


/**
 * @endcond
 **/

DAVIX_C_DECL_BEGIN

using namespace Davix;

struct davix_uri_s;

davix_uri_t davix_uri_new(const char* url){
    return (davix_uri_t) new Uri(url);
}

davix_uri_t davix_uri_copy(davix_uri_t orig_uri){
    g_assert(orig_uri != NULL);
    Uri* myself = (Uri*) orig_uri;
    return (davix_uri_t) new Uri(*myself);
}

void davix_uri_free(davix_uri_t duri){
    if(duri)
        delete ((Uri*) duri);
}

int davix_uri_get_port(davix_uri_t duri){
    g_assert(duri != NULL);
    return ((Uri*) duri)->getPort();
}

const char* davix_uri_get_string(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getString().empty() == false)?(myself->getString().c_str()):NULL);
}

const char* davix_uri_get_path(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getPath().empty() == false)?(myself->getPath().c_str()):NULL);
}

const char* davix_uri_get_host(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getHost().empty() == false)?(myself->getHost().c_str()):NULL);
}

const char* davix_uri_get_path_and_query(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getPathAndQuery().empty() == false)?(myself->getPathAndQuery().c_str()):NULL);
}

const char* davix_uri_get_protocol(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getProtocol().empty() == false)?(myself->getProtocol().c_str()):NULL);
}


davix_status_t davix_uri_get_status(davix_uri_t duri){
    g_assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((davix_status_t) myself->getStatus());
}

DAVIX_C_DECL_END
