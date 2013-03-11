#ifndef DAVIX_HTTPCACHETOKEN_INTERNAL_HPP
#define DAVIX_HTTPCACHETOKEN_INTERNAL_HPP

#include <request/httpcachetoken.hpp>


namespace Davix {


struct HttpCacheTokenAccessor
{

    void addRedirection(HttpCacheToken & token, const Uri & uri);
};


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
