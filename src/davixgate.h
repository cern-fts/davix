#ifndef DAVIXGATE_H
#define DAVIXGATE_H

#include <davixcontext.h>


namespace Davix{

class Context;

class Gate
{
public:
    virtual ~Gate(){};


protected:
    Context* context;
    Gate(Context* context);

    friend class Context;
};


}

#endif // DAVIXGATE_H
