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


HttpRequest* Context::createRequest(const std::string & uri, DavixError** err){
    return _intern->createRequest(uri, err);
}


}


DAVIX_C_DECL_BEGIN


int davix_set_pkcs12_auth(davix_auth_t token, const char* filename_pkcs, const char* passwd, davix_error_t* err){
    Davix::Request* req = (Davix::Request*)(token);
    Davix::DavixError* tmp_err=NULL;

    if( req->try_set_pkcs12_cert(filename_pkcs, passwd, &tmp_err) <0){
        Davix::DavixError::propagateError((Davix::DavixError**) err, tmp_err);
        return -1;
    }


    return 0;
}

int davix_set_login_passwd_auth(davix_auth_t token, const char* login, const char* passwd, davix_error_t* err){
    Davix::Request* req = (Davix::Request*)(token);
    Davix::DavixError* tmp_err=NULL;

    if(req->try_set_login_passwd(login, passwd, &tmp_err) <0){
        Davix::DavixError::propagateError((Davix::DavixError**) err, tmp_err);
        return -1;
    }
    return 0;
}

davix_sess_t davix_context_new(davix_error_t* err){
    Davix::Context* comp = new Davix::Context();
    return (davix_sess_t) comp;

}

davix_sess_t davix_context_copy(davix_sess_t sess){
    return (davix_sess_t) new Davix::Context(*((Davix::Context*) sess));
}



void davix_context_free(davix_sess_t sess){
    if(sess != NULL)
        delete ((Davix::Context*)(sess));
}

DAVIX_C_DECL_END
