#ifndef DAVIX_HTTPCACHETOKEN_INTERNAL_HPP
#define DAVIX_HTTPCACHETOKEN_INTERNAL_HPP

#include <request/httpcachetoken.hpp>


namespace Davix {


struct HttpCacheTokenAccessor
{

    static HttpCacheToken* createCacheToken(const Uri & uri, const Uri & red_uri);
};


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
