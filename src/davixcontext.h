#ifndef DAVIXCONTEXT_H
#define DAVIXCONTEXT_H


namespace Davix{

class PosixGate;

/// @brief Main Entry point for Davix
/// a Davix context is a independant instance of Davix
/// Each instance of Davix has its own session-reuse pool and parameters
class Context
{
public:
    /// create a new context for Davix
    Context();

    virtual ~Context();

    /// clone this instance to a new context dynamically allocated,
    /// the new context inherit of a copy of all the parent context parameters
    /// this context need to be destroyed after usage
    Context* clone();

    /// POSIX-like gate
    /// provide the File POSIX-oriented operations
    /// this gate need to be before the destruction of its context
    PosixGate* posixGate();
protected:

};

}

#endif // DAVIXCONTEXT_H
