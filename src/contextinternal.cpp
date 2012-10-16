#include "contextinternal.h"

#include <neon/neonsessionfactory.hpp>
namespace Davix {

using namespace Glib;

ContextInternal::ContextInternal(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _s_buff = 65536;
    _timeout = 300;
}


AbstractSessionFactory* ContextInternal::getSessionFactory(){
    return _fsess.get();
}

void ContextInternal::set_buffer_size(const size_t value){
    _s_buff = value;
}




} // namespace Davix
