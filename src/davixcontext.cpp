#include <davixcontext.hpp>


#include <contextinternal.h>

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


HttpRequest* Context::createRequest(const std::string & url, DavixError** err){
    return _intern->createRequest(url, err);
}


HttpRequest* Context::createRequest(const Uri &uri, DavixError **err){
    return _intern->createRequest(uri, err);
}

}


DAVIX_C_DECL_BEGIN


int davix_auth_set_pkcs12_cli_cert(davix_auth_t token, const char* filename_pkcs, const char* passwd, davix_error_t* err){
    Davix::NEONRequest* req = (Davix::NEONRequest*)(token);
    Davix::DavixError* tmp_err=NULL;

    if( req->do_pkcs12_cert_authentification(filename_pkcs, passwd, &tmp_err) <0){
        Davix::DavixError::propagateError((Davix::DavixError**) err, tmp_err);
        return -1;
    }

    return 0;
}

int davix_auth_set_login_passwd(davix_auth_t token, const char* login, const char* passwd, davix_error_t* err){
    Davix::NEONRequest* req = (Davix::NEONRequest*)(token);
    Davix::DavixError* tmp_err=NULL;

    if(req->do_login_passwd_authentification(login, passwd, &tmp_err) <0){
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
