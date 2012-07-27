#include "core.hpp"
#include <glibmm.h>

namespace Davix {

using namespace Glib;

Core::Core(AbstractSessionFactory* fsess) : _fsess(fsess)
{
    _s_buff = 65536;
    _timeout = 300;
}


AbstractSessionFactory* Core::getSessionFactory(){
    return _fsess.get();
}

void Core::set_buffer_size(const size_t value){
    _s_buff = value;
}




} // namespace Davix


extern "C"{


int davix_set_pkcs12_auth(davix_auth_t token, const char* filename_pkcs, const char* passwd, GError** err){
    Davix::Request* req = (Davix::Request*)(token);
    try{
        req->try_set_pkcs12_cert(filename_pkcs, passwd);
    }catch(Glib::Error & e){
            g_set_error(err, e.domain(), e.code(), "%s", e.what().c_str());
            return -1;
    }
    return 0;
}

int davix_set_login_passwd_auth(davix_auth_t token, const char* login, const char* passwd, GError** err){
    Davix::Request* req = (Davix::Request*)(token);
    try{
        req->try_set_login_passwd(login, passwd);
    }catch(Glib::Error & e){
            g_set_error(err, e.domain(), e.code(), "%s", e.what().c_str());
            return -1;
    }
    return 0;
}

}
