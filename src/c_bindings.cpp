#include "davixcontext.hpp"
#include <http_backend.hpp>
#include <davixrequestparams.hpp>
#include <glibmm.h>
#include <glib.h>


DAVIX_C_DECL_BEGIN

// initialization
__attribute__((constructor))
void core_init(){
    if (!g_thread_supported())
      g_thread_init(NULL);
    Glib::init();
}








int davix_params_set_auth_callback(davix_params_t params, davix_auth_callback call, void* userdata, davix_error_t* err){
    g_return_val_if_fail(params != NULL, -1);
    Davix::RequestParams* p = (Davix::RequestParams*)(params);
    p->setAuthentificationCallback(userdata, call);
    return 0;
}


int davix_params_set_ssl_check(davix_params_t params, gboolean ssl_check, davix_error_t* err){
    g_return_val_if_fail(params != NULL, -1);
    Davix::RequestParams* p = (Davix::RequestParams*)(params);
    p->setSSLCAcheck(ssl_check);
    return 0;
}

/*

int davix_set_default_session_params(davix_sess_t sess, davix_params_t params, GError ** err){
    g_return_val_if_fail(params != NULL && sess != NULL , -1);
    int ret = -1;
    try{
        Davix::RequestParams* p = (Davix::RequestParams*)(params);
        Davix::CoreInterface* comp = static_cast<Davix::CoreInterface*>(sess);
        comp->getSessionFactory()->set_parameters(*p);
        ret = 0;
    }catch(Glib::Error & e){
        if(err)
            g_error_copy(e.gobj());
    }catch(...){
        g_set_error(err, g_quark_from_string("davix_set_auth_callback"), EINVAL, "unexpected error");
    }
    return ret;
}*/





davix_params_t davix_params_new(){
    return (struct davix_request_params*) new Davix::RequestParams();
}

davix_params_t davix_params_copy(davix_params_t p){
    return (struct davix_request_params*) new Davix::RequestParams(*(Davix::RequestParams*) p);
}

void davix_params_free(davix_params_t p){
    if(p){
        delete ((Davix::RequestParams*) p);
    }
}


DAVIX_C_DECL_END


