#include "core.hpp"
#include <http_backend.hpp>
#include <glibmm.h>

namespace Davix{
//
// Main entry point
//
Composition* session_create(){
    return static_cast<Composition*>(new Core(new NEONSessionFactory() ));
}

}

extern "C"{

// initialization
__attribute__((constructor))
void core_init(){
    g_thread_init(NULL);
    Glib::init();
}



davix_sess_t davix_session_new(GError ** err){
    try{
        Davix::Composition* comp = static_cast<Davix::Composition*>(new Davix::Core(new Davix::NEONSessionFactory() ));
        return (davix_sess_t) comp;
    }catch(Glib::Error & e){
        if(err)
            g_error_copy(e.gobj());
    }catch(...){
        g_set_error(err, g_quark_from_string("davix_session_new"), ENOSYS, "unexpected error");
    }
    return NULL;
}

int davix_stat(davix_sess_t sess, const char* url, struct stat * st, GError** err){
    g_return_val_if_fail(sess != NULL,-1);

    try{
        Davix::Composition* comp = static_cast<Davix::Composition*>(sess);

        comp->stat(url, st);
        return 0;
    }catch(Glib::Error & e){
        if(err)
            *err= g_error_copy(e.gobj());
    }catch(std::exception & e){
        g_set_error(err, g_quark_from_string("davix_stat"), EINVAL, "unexcepted error %s", e.what());
    }
    return -1;
}



int davix_set_auth_callback(davix_sess_t sess, davix_auth_callback call, void* userdata, GError** err){
    g_return_val_if_fail(sess != NULL, -1);
    int ret = -1;
    try{
        Davix::Composition* comp = static_cast<Davix::Composition*>(sess);
        comp->getSessionFactory()->set_authentification_controller(userdata, call);
        ret = 0;
    }catch(Glib::Error & e){
        if(err)
            g_error_copy(e.gobj());
    }catch(...){
        g_set_error(err, g_quark_from_string("davix_set_auth_callback"), EINVAL, "unexpected error");
    }
    return ret;
}

int davix_set_ssl_check(davix_sess_t sess, gboolean ssl_check, GError** err){
    g_return_val_if_fail(sess != NULL, -1);
    int ret = -1;
    try{
        Davix::Composition* comp = static_cast<Davix::Composition*>(sess);
        comp->getSessionFactory()->set_ssl_ca_check(ssl_check);
        ret = 0;
    }catch(Glib::Error & e){
        if(err)
            g_error_copy(e.gobj());
    }catch(...){
        g_set_error(err, g_quark_from_string("davix_set_auth_callback"), EINVAL, "unexpected error");
    }
    return ret;
}



void davix_session_free(davix_sess_t sess){
    if(sess != NULL)
        delete static_cast<Davix::Composition*>(sess);
}


}
