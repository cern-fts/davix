#include "davixcontext.h"

#include <posixgate.h>
#include <httpgate.h>
#include <contextinternal.h>
#include <glibmm.h>

namespace Davix{

Context::Context() : _intern(new ContextInternal())
{
}

Context::Context(const Context &c) : ContextConfig(c){
    this->_intern = c._intern;
}

Context::~Context(){

}

Context* Context::clone(){

    return new Context(*this);
}


PosixGate& Context::posixGate(){
    if(p_gate.get() == NULL){
        Glib::Mutex::Lock l(mux_gate);
        if(p_gate.get() == NULL){
            p_gate.reset(new PosixGate(this));
        }
    }
    return *(p_gate);
}

HttpGate& Context::httpGate(){
    if(h_gate.get() == NULL){
        Glib::Mutex::Lock l(mux_gate);
        if(h_gate.get() == NULL){
            h_gate.reset(new HttpGate(this));
        }
    }
    return *(h_gate);
}

}
