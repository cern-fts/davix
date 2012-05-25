#include "core.h"
#include <glibmm/init.h>

namespace Davix {

using namespace Glib;

Core::Core(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _s_buff = 65536;
    _timeout = 10;
}

RefPtr<Core> Core::create(AbstractSessionFactory* fsess){
    return Glib::RefPtr<Core>(new Core(fsess));
}




AbstractSessionFactory* Core::getSessionFactory(){
    return _fsess.get();
}

void Core::set_buffer_size(const size_t value){
    _s_buff = value;
}




} // namespace Davix


extern "C"{

// glibmm initialization
__attribute__((constructor))
void core_init(){
    g_thread_init(NULL);
    Glib::init();
}




}
