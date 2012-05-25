#include "core.h"
#include <glibmm/init.h>

namespace Davix {

using namespace Glib;

Core::Core(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _s_buff = 65536;
    _timeout = 300;
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


int davix_set_pkcs12_auth(davix_auth_t token, const char* filename_pkcs12,const char* passwd, GError** err){
    Davix::Request* req = static_cast<Davix::Request*>(token);
    try{
        req->try_set_pkcs12_cert(filename_pkcs12, passwd);
    }catch(Glib::Error & e){
            g_set_error(err, e.domain(), e.code(), e.what().c_str());
            return -1;
    }
    return 0;
}

}
