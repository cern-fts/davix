#include <iterator>
#include "httpcachetoken_internal.hpp"



namespace Davix {

struct HttpCacheTokenInternal{

    HttpCacheTokenInternal() :
        _uris()
    {}

    HttpCacheTokenInternal(const HttpCacheTokenInternal & orig){
        _uris.reserve(orig._uris.size());
        std::copy(orig._uris.begin(), orig._uris.end(), _uris.begin());
    }

    virtual ~HttpCacheTokenInternal(){}



    std::vector<Uri> _uris;
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


const std::vector<Uri>  & HttpCacheToken::getRedirectionStack() const{
    return d_ptr->_uris;
}

void HttpCacheTokenAccessor::addRedirection(HttpCacheToken & token, const Uri & uri){
    token.d_ptr->_uris.push_back(uri);
}

}

