#include "core.h"
#include <curl/curlrequest.h>
#include <curl/curlsessionfactory.h>
#include <glibmm/init.h>

extern "C"{

davix_sess_t davix_session_new(GError ** err){
    try{
        Davix::Composition* comp = static_cast<Davix::Composition*>(new Davix::Core(new Davix::CURLSessionFactory() ));
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


void davix_session_free(davix_sess_t sess){
    if(sess != NULL)
        delete static_cast<Davix::Composition*>(sess);
}


}
