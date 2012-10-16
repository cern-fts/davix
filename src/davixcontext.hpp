#ifndef DAVIXCONTEXT_H
#define DAVIXCONTEXT_H

#include <string>

namespace Davix{

class ContextInternal;
class HttpRequest;

/// @brief Main Entry point for Davix
/// a Davix context is a independant instance of Davix
/// Each instance of Davix has its own session-reuse pool and parameters
class Context
{
public:
    /// create a new context for Davix
    Context();
    Context(const Context & c);

    virtual ~Context();

    /// clone this instance to a new context dynamically allocated,
    /// the new context inherit of a copy of all the parent context parameters
    /// this context need to be destroyed after usage
    Context* clone();


    /// create a new Http request for direct HTTP low level feature usage
    /// this Http request object should be destroyed after usage
    HttpRequest* createRequest(const std::string & uri);


private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;

};

}

#endif // DAVIXCONTEXT_H
