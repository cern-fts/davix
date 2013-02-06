#ifndef DAVIXCONTEXT_HPP
#define DAVIXCONTEXT_HPP

#include <string>
#include <davix_types.h>
#include <status/davixstatusrequest.hpp>

#include <davixuri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


///
/// @file davixcontext.hpp
///

namespace Davix{

struct ContextInternal;
struct ContextExplorer;
class HttpRequest;
class DavPosix;


/// @brief Main entry point for Davix
/// Each new davix context has its own session-reuse pool and set of parameters
/// Any Method in a Davix::Context is thread-safe
class Context
{
public:
    /// create a new context for Davix
    Context();
    Context(const Context & c);
    Context & operator=(const Context & c);

    virtual ~Context();

    /// clone this instance to a new context dynamically allocated,
    /// the new context inherit of a copy of all the parent context parameters
    /// this context need to be destroyed after usage
    Context* clone();


    /// create a new Http request for direct HTTP low level feature usage
    /// this HTTP request object should be destroyed after usage
    HttpRequest* createRequest(const std::string & url, DavixError** err);
    HttpRequest* createRequest(const Uri & uri, DavixError** err);

    /// create a posix entry point from this context
    /// this object is thread-safe
    /// need to be delete
    DavPosix* createDavPosix();

private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;
    friend class ContextExplorer;
};


const std::string & version();

}

#endif // DAVIXCONTEXT_HPP
