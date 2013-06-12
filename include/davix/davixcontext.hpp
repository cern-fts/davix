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
    /// the new context inherit of a copy of all the parent context parameters
    /// this context need to be destroyed after usage
    /// @return new allocated clone of this context
    Context* clone();


    /// @brief low level operations
    ///
    /// create a new Http request for direct HTTP low level feature usage
    /// this HTTP request object should be destroyed after usage
    ///
    /// This function is thread safe.
    ///     several requests object can be used on the same context in parallel
    ///
    /// @param uri : Davix \ref Uri to use for the request
    /// @param err : Davix Error report
    /// @return pointer to a new allocated request object or null if error
    HttpRequest* createRequest(const Uri & uri, DavixError** err);

    /// @brief low level operations
    ///
    ///  similar to \ref createRequest(const Uri & uri, DavixError** err) but with
    ///  a raw string input
    ///
    /// @param url : url to use for the request
    /// @param err : Davix Error report
    /// @return pointer to a new allocated request object or null if error
    HttpRequest* createRequest(const std::string & url, DavixError** err);

    /// @brief POSIX-like operations
    ///
    /// Create a new allocated \ref DavPosix Object entry point
    ///
    /// \ref DavPosix is the main entry point for all the POSIX-like operation :
    ///
    ///     ex : stat()
    ///          open() / read() / write() / close()
    ///          opendir() / readdir() / closedir()
    ///          mkdir() / rmdir() / unlink()
    ///
    /// These operations follows the POSIX semantic as much as possible.
    ///
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
