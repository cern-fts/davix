#include <config.h>
#include <davixuri.hpp>
#include <ne_uri.h>
#include <malloc.h>
#include <cassert>
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
        query_and_path(),
        _uri_string(){

        memset(&my_uri, 0, sizeof(my_uri));
    }

    UriPrivate(const UriPrivate & orig):
        my_uri(),
        code(orig.code),
        proto(orig.proto),
        path(orig.path),
        host(orig.host),
        query(orig.query),
        query_and_path(orig.query_and_path),
        _uri_string(orig._uri_string){
        ne_uri_copy(&my_uri, &(orig.my_uri));

    }

    ~UriPrivate() {
        ne_uri_free(&my_uri);
    }

    void parsing(const std::string & uri_string){
        _uri_string = uri_string;
        if(ne_uri_parse(uri_string.c_str(), &(my_uri)) == 0){
            if(my_uri.scheme == NULL
               || my_uri.path == NULL
               || my_uri.host ==NULL)
                return;

            if(my_uri.port == UINT_MAX)
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
    std::string _uri_string;

};

Uri::Uri() :
    d_ptr(new UriPrivate()){
}


Uri::Uri(const std::string & uri) :
    d_ptr(new UriPrivate()){
    d_ptr->parsing(uri);
}

Uri::Uri(const Uri & uri) :
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
    return d_ptr->_uri_string;
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

bool Uri::equal(const Uri &u2) const{
    if(this->getStatus() != Davix::StatusCode::OK || u2.getStatus() != Davix::StatusCode::OK)
        return false;
    return ne_uri_cmp(&this->d_ptr->my_uri, &u2.d_ptr->my_uri) ==0;
}

bool Uri::operator ==(const Uri & u2) const{
    return this->equal(u2);
}


std::string Uri::escapeString(const std::string & str){
    char* p = ne_path_escape(str.c_str());
    std::string  res(p);
    free(p);
    return res;
}

std::string Uri::unescapeString(const std::string & str){
    char* p = ne_path_unescape(str.c_str());
    std::string res(p);
    free(p);
    return res;
}

bool uriCheckError(const Uri &uri, DavixError **err){
    if(uri.getStatus() == StatusCode::OK)
        return true;
    DavixError::setupError(err, davix_scope_uri_parser(), uri.getStatus(), std::string("Uri syntax Invalid : ").append(uri.getString()));
    return false;
}



unsigned int httpUriGetPort(const Uri & uri){
    int port = uri.getPort();
    if( port ==0){
        const std::string & scheme = uri.getProtocol();
        if(*scheme.rbegin() == 's') // davs, https or s3s
            port = 443;
        else
            port =80;
    }
    return port;
}


} // namespace Davix


std::ostream& operator<< (std::ostream& stream, const Davix::Uri & _u){
	stream << _u.getString();
	return stream;
}


