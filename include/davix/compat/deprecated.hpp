#ifndef DAVIX_DEPRECATED
#define DAVIX_DEPRECATED

#include <vector>
#include <davix_types.h>
#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

/**
  @file deprecated.hpp
  @author Devresse Adrien

  @brief Deprecated class / functions of Davix for ABI/ API compatibility

   Each call to one of these function kill a kitten.
   Please love kittens and don't use them
 */




namespace Davix {

struct HttpCacheTokenInternal;


class DAVIX_EXPORT HttpCacheToken
{
public:

    HttpCacheToken();

    HttpCacheToken(const HttpCacheToken & orig);

    virtual ~HttpCacheToken();

    HttpCacheToken & operator=(const HttpCacheToken &);

    const Uri & getrequestUri() const;


    const Uri & getCachedRedirection() const;
private:
    HttpCacheTokenInternal* d_ptr;

    friend struct HttpCacheTokenAccessor;
};


} // namespace Davix

#endif
