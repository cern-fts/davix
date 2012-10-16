#ifndef DAVIXCONTEXT_H
#define DAVIXCONTEXT_H



namespace Davix{

class PosixGate;
class HttpGate;
class ContextInternal;

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


private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;

};

}

#endif // DAVIXCONTEXT_H
