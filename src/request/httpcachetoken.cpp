#include <davix_internal.hpp>
#include "httpcachetoken_internal.hpp"



namespace Davix {

struct HttpCacheTokenInternal{

    HttpCacheTokenInternal() :
        _req_uri(),
        _redirection_uri()
    {}

    HttpCacheTokenInternal(const HttpCacheTokenInternal & orig) :
        _req_uri(orig._req_uri),
        _redirection_uri(orig._redirection_uri)

    {

    }

    virtual ~HttpCacheTokenInternal(){}



    Uri _req_uri, _redirection_uri;
private:
    HttpCacheTokenInternal & operator=(const HttpCacheTokenInternal &);
};


HttpCacheToken::HttpCacheToken() :
    d_ptr(new HttpCacheTokenInternal())
{

}

HttpCacheToken::HttpCacheToken(const HttpCacheToken &orig) :
    d_ptr(new HttpCacheTokenInternal(*(orig.d_ptr)))
{

}

HttpCacheToken & HttpCacheToken::operator=(const HttpCacheToken & orig){
    if(&orig == this)
        return *this;
    delete d_ptr;
    d_ptr = new HttpCacheTokenInternal(*(orig.d_ptr));
    return *this;
}

HttpCacheToken::~HttpCacheToken(){
    delete d_ptr;
}


const Uri  & HttpCacheToken::getCachedRedirection() const{
    return d_ptr->_redirection_uri;
}

const Uri & HttpCacheToken::getrequestUri() const {
    return d_ptr->_req_uri;
}

HttpCacheToken* HttpCacheTokenAccessor::createCacheToken(const Uri & uri, const Uri & red_uri){
    HttpCacheToken* t = new HttpCacheToken();
    t->d_ptr->_req_uri = uri;
    t->d_ptr->_redirection_uri = red_uri;
    return t;
}
}

