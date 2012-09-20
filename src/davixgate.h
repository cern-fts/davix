#ifndef DAVIXGATE_H
#define DAVIXGATE_H

#include <davixcontext.h>


namespace Davix{

class Context;

class Gate
{
public:
    Gate(Context* context);
    virtual ~Gate(){};


protected:
    Context* context;
};


}

#endif // DAVIXGATE_H
