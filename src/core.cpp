#include "core.h"
#include <glibmm/init.h>

namespace Davix {

using namespace Glib;

Core::Core(AbstractSessionFactory* fsess) : _fsess(fsess)
{
}

RefPtr<Core> Core::create(AbstractSessionFactory* fsess){
    return Glib::RefPtr<Core>(new Core(fsess));
}

Glib::RefPtr<Stat> Core::getStat(){
    return Glib::RefPtr<Stat>(new Stat(this, _fsess.get()));
}


AbstractSessionFactory* Core::getSessionFactory(){
    return _fsess.get();
}


} // namespace Davix


extern "C"{

// glibmm initialization
__attribute__((constructor))
void core_init(){
    Glib::init();
}




}
