#include "neonsession.hpp"

#include <string>
#include <cstring>

#include <davix_context_internal.hpp>
#include <ne_redirect.h>
#include <libs/time_utils.h>


namespace Davix{


static int validate_all_certificate(void *userdata, int failures,
                                const ne_ssl_certificate *cert){
    return 0;
}




void NEONSession::provide_clicert_fn(void *userdata, ne_session *sess,
                                         const ne_ssl_dname *const *dnames,
                                         int dncount){

    NEONSession* req = static_cast<NEONSession*>(userdata);
    DavixError* tmp_err=NULL;
    davix_auth_info_t auth_info;
    auth_info.auth = DAVIX_CLI_CERT_PKCS12;
    davix_auth_callback auth_call = req->_params.getAuthentificationCallbackFunction();

    davix_log_debug("NEONSession > clicert callback ");
    if( auth_call == NULL){
        davix_log_debug("NEONSession : No callback specified, cancel authentification");
        return;
    }else{
        davix_log_debug("NEONSession > call authentification callback ");
        int ret = auth_call((davix_auth_t) req, &auth_info, req->_params.getAuthentificationCallbackData(), (davix_error_t*) &tmp_err); // try to get authentification
        davix_log_debug("NEONSession > return from authentification callback ");
        if(ret !=0 && tmp_err == NULL){
            DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::authentificationError, "Authentification callback returned with CANCEL without DavixError object");
        }
    }
    if(tmp_err){
       DavixError::propagateError(&(req->_last_error), tmp_err);
    }
    return;
}

int NEONSession::provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password){
    NEONSession * req = static_cast<NEONSession*>(userdata);

     davix_log_debug("NEONRequest > Try to get auth/password authentification ");
     davix_auth_info_t auth_info;
    // memset(&auth_info,0,sizeof(davix_auth_info_t));
     davix_auth_callback auth_call = req->_params.getAuthentificationCallbackFunction();
     auth_info.auth = DAVIX_LOGIN_PASSWORD;
     DavixError* tmp_err=NULL;

     if(auth_call  == NULL){
         davix_log_debug("NEONSession : No credential specified, cancel login/password authentification");
         return -1;
     }

     davix_log_debug("NEONSession > call authentification callback ");
     int ret = auth_call((davix_auth_t) req, &auth_info, req->_params.getAuthentificationCallbackData(), (davix_error_t*) &tmp_err); // try to get authentification
     davix_log_debug("NEONSession > return from authentification callback ");
     if(ret != 0){
            DavixError::propagateError(&(req->_last_error), tmp_err);
            return -2;
     }

     if( req->_passwd.empty()
        || req->_login.empty() ){
        davix_log_debug("NEONSession > Login/Password missings ....");
        return -1;
    }
    davix_log_debug("NEONSession > setup authentification pwd/login....");
    g_strlcpy(username, req->_login.c_str(), NE_ABUFSIZ);
    g_strlcpy(password, req->_passwd.c_str(), NE_ABUFSIZ);
    req->_login.clear();
    req->_passwd.clear();
    return 0;

}



NEONSession::NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err) :
    _f(ContextExplorer::SessionFactoryFromContext(c)),
    _sess(NULL),
    _params(p),
    _last_error(NULL),
    _login(),
    _passwd()
{
        _f.createNeonSession(uri, &_sess, err);
        if(_sess)
            configureSession(_sess, p, &NEONSession::provide_login_passwd_fn, this, &NEONSession::provide_clicert_fn, this);
}

NEONSession::~NEONSession(){
    if(_sess)
        _f.storeNeonSession(_sess, NULL);
}


ne_session* NEONSession::get_ne_sess(){
    return _sess;
}


void configureSession(ne_session *_sess, const RequestParams &params, ne_auth_creds lp_callback, void* lp_userdata,
                      ne_ssl_provide_fn cred_callback,  void* cred_userdata){
    if(strcmp(ne_get_scheme(_sess), "https") ==0) // fix a libneon bug with non ssl connexion
        ne_ssl_trust_default_ca(_sess);

    // register redirection management
    ne_redirect_register(_sess);
    //ne_set_session_flag(_sess, NE_SESSFLAG_PERSIST, false);

    // define user agent
    ne_set_useragent(_sess, params.getUserAgent().c_str());

    if(params.getSSLCACheck() == false){ // configure ssl check
        davix_log_debug("NEONRequest : disable ssl verification");
        ne_ssl_set_verify(_sess, validate_all_certificate, NULL);
    }

    // if authentification callback defined, enable the wrapper
    if( params.getAuthentificationCallbackFunction() != NULL){
        ne_ssl_provide_clicert(_sess, cred_callback, lp_userdata);
        ne_set_server_auth(_sess, lp_callback, cred_userdata);
    }



    if( timespec_isset(params.getOperationTimeout())){
        davix_log_debug("NEONSession : define operation timeout to %d", params.getOperationTimeout());
        ne_set_read_timeout(_sess, (int) params.getOperationTimeout()->tv_sec);
    }
    if(timespec_isset(params.getConnexionTimeout())){
        davix_log_debug("NEONSession : define connexion timeout to %d", params.getConnexionTimeout());
#ifndef _NEON_VERSION_0_25
        ne_set_connect_timeout(_sess, (int) params.getConnexionTimeout()->tv_sec);
#endif
    }

}





}
