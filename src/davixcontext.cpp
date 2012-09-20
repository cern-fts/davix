#include "davixcontext.h"

#include <posixgate.h>
#include <httpgate.h>

namespace Davix{

Context::Context()
{
}

Context::~Context(){

}

Context* Context::clone(){
    return new Context();
}


PosixGate* Context::posixGate(){
    return new PosixGate(this);
}

HttpGate* Context::httpGate(){
    return new HttpGate(this);
}

}
