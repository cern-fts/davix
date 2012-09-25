#include "davixcontext.h"

#include <posixgate.h>
#include <httpgate.h>
#include <contextinternal.h>

namespace Davix{

Context::Context() : _intern(new ContextInternal())
{
}

Context::Context(const Context &c){
    this->_intern = c._intern;
}

Context::~Context(){

}

Context* Context::clone(){
    return new Context(*this);
}


PosixGate* Context::posixGate(){
    return new PosixGate(this);
}

HttpGate* Context::httpGate(){
    return new HttpGate(this);
}

}
