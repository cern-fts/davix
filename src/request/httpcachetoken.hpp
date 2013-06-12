#ifndef DAVIX_HTTPCACHETOKEN_HPP
#define DAVIX_HTTPCACHETOKEN_HPP

#include <vector>
#include <davix_types.h>
#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

/**
  @file httpcachetoken.hpp
  @author Devresse Adrien

  @brief Request cache control feature of davix
 */




namespace Davix {

struct HttpCacheTokenInternal;

/// @class HttpCacheToken
/// @brief State of the cache of a past HTTPRequest
/// transmit a \ref HttpCacheToken object enable speed optimizations
///
class DAVIX_EXPORT HttpCacheToken
{
public:
    ///
    /// \brief default constructor
    ///
    HttpCacheToken();
    ///
    /// \brief copy constructor
    /// \param orig
    ///
    HttpCacheToken(const HttpCacheToken & orig);
    ///
    /// \brief descruptor
    ///
    virtual ~HttpCacheToken();
    ///
    /// \brief assignment operator
    /// \return
    HttpCacheToken & operator=(const HttpCacheToken &);

    ///
    /// \brief uri of the request associated with this cache token
    /// \return
    ///
    const Uri & getrequestUri() const;

    ///
    /// \brief redirection cache
    /// \return get the final redirection Uri associated with the request Uri
    ///
    const Uri & getCachedRedirection() const;
private:
    HttpCacheTokenInternal* d_ptr;

    friend struct HttpCacheTokenAccessor;
};


} // namespace Davix

#endif // DAVIX_HTTPCACHETOKEN_HPP
