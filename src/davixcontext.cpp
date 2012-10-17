#include <davixcontext.hpp>


#include <contextinternal.h>
#include <glibmm.h>

namespace Davix{

Context::Context()
{
    _intern= new ContextInternal(new NEONSessionFactory());
}

Context::Context(const Context &c){
    this->_intern = ContextInternal::takeRef(c._intern);
}

Context::~Context(){
    ContextInternal::releaseRef(_intern);

}

Context* Context::clone(){

    return new Context(*this);
}


HttpRequest* Context::createRequest(const std::string & uri){
    return _intern->createRequest(uri);
}


}


DAVIX_C_DECL_BEGIN


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

davix_sess_t davix_context_new(GError ** err){
    try{
        Davix::Context* comp = new Davix::Context();
        return (davix_sess_t) comp;
    }catch(Glib::Error & e){
        if(err)
            g_error_copy(e.gobj());
    }catch(...){
        g_set_error(err, g_quark_from_string("davix_context_new"), ENOSYS, "unexpected error");
    }
    return NULL;
}

davix_sess_t davix_context_copy(davix_sess_t sess){
    return (davix_sess_t) new Davix::Context(*((Davix::Context*) sess));
}



void davix_context_free(davix_sess_t sess){
    if(sess != NULL)
        delete ((Davix::Context*)(sess));
}

DAVIX_C_DECL_END
