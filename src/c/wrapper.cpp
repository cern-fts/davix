
#include <davix.hpp>


DAVIX_C_DECL_BEGIN

using namespace Davix;

///  @brief clear a davix error object and release its memory, and set the error pointer to NULL
///
void davix_error_clear(davix_error_t* ptr_err){
    DavixError::clearError((DavixError**) ptr_err);
}

/// @brief create a new davix error object
///
void davix_error_setup(davix_error_t* ptr_err, const char* scope, int status_code, const char* msg){
    DavixError::setupError((DavixError**) ptr_err, scope, (Davix::StatusCode::Code)status_code, msg);
}


const char* davix_error_msg(davix_error_t err){
    return ((DavixError*) err)->getErrMsg().c_str();
}

int davix_error_code(davix_error_t err){
    return ((DavixError*) err)->getStatus();
}

const char* davix_error_scope(davix_error_t err){
    return NULL; // TODO
}

void davix_error_propagate(davix_error_t* newErr, davix_error_t oldErr ){
    DavixError::propagateError((DavixError**)newErr, (DavixError*) oldErr);
}


DAVIX_DIR* davix_posix_opendir(davix_sess_t sess, davix_params_t _params, const char* url,  davix_error_t* err){
    davix_return_val_if_fail(sess != NULL, NULL);
    DAVIX_DIR* ret = NULL;


    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);
    ret= p.opendir(params,url, (Davix::DavixError**) err);

    return ret;
}



int davix_posix_closedir(davix_sess_t sess, DAVIX_DIR* d, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));


    return p.closedir(d,(Davix::DavixError**) err);

}


struct dirent* davix_posix_readdir(davix_sess_t sess, DAVIX_DIR* d, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL,NULL);

    struct dirent* ret = NULL;
    Davix::DavPosix p((Davix::Context*)(sess));

    if(d){
        ret= p.readdir(d, (Davix::DavixError**) err);
    }

    return ret;
}


int davix_posix_mkdir(davix_sess_t sess, davix_params_t _params, const char* url,  mode_t right, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.mkdir(params,url, right, (Davix::DavixError**) err);
}


int davix_posix_unlink(davix_sess_t sess, davix_params_t _params, const char* url,   davix_error_t* err){
    davix_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.unlink(params, url,  (Davix::DavixError**) err);
}

int davix_posix_rmdir(davix_sess_t sess, davix_params_t _params, const char* url,   davix_error_t* err){
    davix_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.rmdir(params, url,  (Davix::DavixError**) err);
}

int davix_posix_stat(davix_sess_t sess, davix_params_t _params, const char* url, struct stat * st, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.stat(params,url, st, (Davix::DavixError**) err);

}

////////////////////////////////////////////////
////////////////////////////////////////////////
///// C bindings
////////////////////////////////////////////////
////////////////////////////////////////////////




int davix_params_set_ssl_check(davix_params_t params, bool ssl_check, davix_error_t* err){
    davix_return_val_if_fail(params != NULL, -1);
    Davix::RequestParams* p = (Davix::RequestParams*)(params);
    p->setSSLCAcheck(ssl_check);
    return 0;
}


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

bool davix_params_get_keep_alive(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getKeepAlive();
}


/// enable or disable http keep alive
void davix_params_set_keep_alive(davix_params_t params, bool keep_alive){
    assert(params);
    ((Davix::RequestParams*) params)->setKeepAlive(keep_alive);
}

void davix_params_set_protocol(davix_params_t params, davix_request_protocol_t protocol){
    assert(params);
    ((Davix::RequestParams*) params)->setProtocol(protocol);
}

davix_request_protocol_t davix_params_get_protocol(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getProtocol();
}


void davix_params_set_trans_redirect(davix_params_t params, bool redirection){
    assert(params);
    ((Davix::RequestParams*) params)->setTransparentRedirectionSupport(redirection);
}

bool davix_params_get_trans_redirect(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getTransparentRedirectionSupport();
}

void davix_params_set_user_agent(davix_params_t params, const char* user_agent){
    assert(params && user_agent);
    ((Davix::RequestParams*) params)->setUserAgent(user_agent);
}


const char* davix_params_get_user_agent(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getUserAgent().c_str();
}


void davix_params_set_conn_timeout(davix_params_t params, unsigned int timeout){
    assert(params);
    struct timespec t;
    t.tv_sec = timeout;
    t.tv_nsec =0;
    return ((Davix::RequestParams*) params)->setConnectionTimeout(&t);
}

unsigned int davix_params_get_conn_timeout(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getConnectionTimeout()->tv_sec;
}



void davix_params_set_ops_timeout(davix_params_t params, unsigned int timeout){
    assert(params);
    struct timespec t;
    t.tv_sec = timeout;
    t.tv_nsec =0;
    return ((Davix::RequestParams*) params)->setOperationTimeout(&t);
}

unsigned int davix_params_get_ops_timeout(davix_params_t params){
    assert(params);
    return ((Davix::RequestParams*) params)->getOperationTimeout()->tv_sec;
}


void davix_params_set_client_cert_X509(davix_params_t params, davix_x509_cert_t cred){
    assert(params && cred);
    ((Davix::RequestParams*) params)->setClientCertX509(*(Davix::X509Credential*) cred);
}


davix_x509_cert_t  davix_params_get_client_cert_X509(davix_params_t params){
    assert(params);
    return (davix_x509_cert_t) &((Davix::RequestParams*) params)->getClientCertX509();
}

/// set login/password for HTTP Authentication
void davix_params_set_login_passwd(davix_params_t params, const char* login, const char*  password){
    assert(params && login && password);
    ((Davix::RequestParams*) params)->setClientLoginPassword(login, password);
}



davix_x509_cert_t davix_x509_cert_new(){
    return (davix_x509_cert_t) new Davix::X509Credential();
}

/// return true if certificate container contain a valid credential, else false
bool davix_x509_cert_has_cert(davix_x509_cert_t cred){
    assert(cred);
    return ((Davix::X509Credential*) cred)->hasCert();
}

/// load a pkcs12 certificate
int davix_x509_cert_load_from_p12(davix_x509_cert_t cred, const char * path, const char* passwd, davix_error_t* err){
    assert(cred && path);
    return ((Davix::X509Credential*) cred)->loadFromFileP12(path, ((passwd)?(passwd):""), (Davix::DavixError**)err);
}

/// free a container for X509 certificate
void davix_x509_cert_free(davix_x509_cert_t cred){
    if(cred)
        delete (Davix::X509Credential*) cred;
}


/**
 * @endcond
 **/


using namespace Davix;

struct davix_uri_s;

davix_uri_t davix_uri_new(const char* url){
    return (davix_uri_t) new Uri(url);
}

davix_uri_t davix_uri_copy(davix_uri_t orig_uri){
    assert(orig_uri != NULL);
    Uri* myself = (Uri*) orig_uri;
    return (davix_uri_t) new Uri(*myself);
}

void davix_uri_free(davix_uri_t duri){
    if(duri)
        delete ((Uri*) duri);
}

int davix_uri_get_port(davix_uri_t duri){
    assert(duri != NULL);
    return ((Uri*) duri)->getPort();
}

const char* davix_uri_get_string(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getString().empty() == false)?(myself->getString().c_str()):NULL);
}

const char* davix_uri_get_path(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getPath().empty() == false)?(myself->getPath().c_str()):NULL);
}

const char* davix_uri_get_host(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getHost().empty() == false)?(myself->getHost().c_str()):NULL);
}

const char* davix_uri_get_path_and_query(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getPathAndQuery().empty() == false)?(myself->getPathAndQuery().c_str()):NULL);
}

const char* davix_uri_get_protocol(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((myself->getProtocol().empty() == false)?(myself->getProtocol().c_str()):NULL);
}


davix_status_t davix_uri_get_status(davix_uri_t duri){
    assert(duri != NULL);
    Uri* myself = (Uri*) duri;
    return ((davix_status_t) myself->getStatus());
}



DAVIX_C_DECL_END

