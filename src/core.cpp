#include "core.h"
#include <glibmm/init.h>

namespace Davix {

using namespace Glib;

Core::Core(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _grid_mode = false;
}

RefPtr<Core> Core::create(AbstractSessionFactory* fsess){
    return Glib::RefPtr<Core>(new Core(fsess));
}

Glib::RefPtr<Stat> Core::getStat(){
    return Glib::RefPtr<Stat>(new Stat(this, _fsess.get()));
}


void Core::set_grid_mode(const int state){
    _grid_mode = state;
}

bool Core::get_grid_mode(){
    return _grid_mode;
}


} // namespace Davix


extern "C"{

// glibmm initialization
__attribute__((constructor))
void core_init(){
    Glib::init();
}




}
