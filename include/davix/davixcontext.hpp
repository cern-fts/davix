#ifndef DAVIXCONTEXT_HPP
#define DAVIXCONTEXT_HPP

#include <string>
#include <status/davixstatusrequest.hpp>

#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


///
/// @file davixcontext.hpp
/// @author Devresse Adrien
///
///  Handle of Davix

namespace Davix{

struct ContextInternal;
struct ContextExplorer;
class HttpRequest;
class DavPosix;


/// @brief Main handle for Davix
///
/// Each new davix context contains its own session-reuse pool and set of parameters
/// a Context can execute multiple queries in parallels and is thread safe
class DAVIX_EXPORT Context
{
public:
    ///
    /// \brief Default constructor
    ///
    Context();
    ///
    /// \brief copy constructor
    /// \param c
    ///
    Context(const Context & c);
    ///
    /// \brief assignment operator
    /// \param c
    /// \return
    ///
    Context & operator=(const Context & c);
    ///
    /// \brief destructor
    ///
    virtual ~Context();

    /// clone this instance to a new context dynamically allocated,
    Context* clone();



    ///  enable or disablet the session caching
    void setSessionCaching(bool caching);

    bool getSessionCaching() const;


    /// @deprecated
    HttpRequest* createRequest(const Uri & uri, DavixError** err);
    /// @deprecated
    HttpRequest* createRequest(const std::string & url, DavixError** err);
    /// @deprecated
    DavPosix* createDavPosix();

private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;
    friend struct ContextExplorer;
};


/// version string of the current davix library
/// @return version of davix
DAVIX_EXPORT const std::string & version();

}

#endif // DAVIXCONTEXT_HPP
