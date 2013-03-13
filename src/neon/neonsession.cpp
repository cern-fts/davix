#include "neonsession.hpp"


#include <config.h>
#include <string>
#include <cstring>
#include <logger/davix_logger_internal.h>
#include <davix_context_internal.hpp>
#include <ne_redirect.h>
#include <libs/time_utils.h>
#include <auth/davixx509cred_internal.hpp>


namespace Davix{


static int validate_all_certificate(void *userdata, int failures,
                                const ne_ssl_certificate *cert){
    return 0;
}

const int n_max_auth = 20;


void NEONSession::provide_clicert_fn(void *userdata, ne_session *sess,
                                         const ne_ssl_dname *const *dnames,
                                         int dncount){

    NEONSession* req = static_cast<NEONSession*>(userdata);
    DavixError* tmp_err=NULL;

    X509Credential cert;
    std::pair<authCallbackClientCertX509,void*> retcallback = req->_params.getClientCertCallbackX509();
    DAVIX_DEBUG("NEONSession > clicert callback ");
    if( retcallback.first != NULL){
        DAVIX_DEBUG("NEONSession > call client cert callback ");
        SessionInfo infos;

        if( retcallback.first(retcallback.second, infos, &cert, &tmp_err) != 0 || cert.hasCert() == false){
            if(!tmp_err)
                DavixError::setupError(&(req->_last_error), davix_scope_http_request(), StatusCode::AuthentificationError,
                                       "No valid credential given ");
             return;
        }

        ne_ssl_set_clicert(req->_sess, X509CredentialExtra::extract_ne_ssl_clicert(cert));
        DAVIX_DEBUG("NEONSession > end call client cert callback");
    }
    return;
}

int NEONSession::provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password){
    NEONSession * req = static_cast<NEONSession*>(userdata);
    DavixError * tmp_err=NULL;
    int ret =-1;


    DAVIX_DEBUG("NEONSession > Try to get auth/password authentification from client");

     if(attempt > n_max_auth ){
         DavixError::setupError(&(req->_last_error), davix_scope_http_request(), StatusCode::LoginPasswordError,
                                "Overpass allowed number of authentication attempt");
     }

     const std::pair<authCallbackLoginPasswordBasic, void*> retcallback(req->_params.getClientLoginPasswordCallback());
     const std::pair<std::string , std::string > id(req->_params.getClientLoginPassword());
     if(retcallback.first != NULL){
         DAVIX_DEBUG("NEONSession > Try callback for login/passwd for %d time", attempt+1);
         SessionInfo infos;
         std::string tmp_login, tmp_password;
         if( (ret = retcallback.first(retcallback.second, infos, tmp_login, tmp_password, attempt, &tmp_err) ) <0){
             if(!tmp_err)
                 DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::LoginPasswordError,
                                        "No valid login/passwd");
              DavixError::propagateError(&(req->_last_error), tmp_err);
              return -1;
         }
         strlcpy(username, tmp_login.c_str(), NE_ABUFSIZ);
         strlcpy(password, tmp_password.c_str(), NE_ABUFSIZ);
     }else if(id.first.empty() == false){
        strlcpy(username, id.first.c_str(), NE_ABUFSIZ);
        strlcpy(password, id.second.c_str(), NE_ABUFSIZ);
     }

    DAVIX_DEBUG("NEONSession > get login/password with success...try server submission ");
    return 0;

}



NEONSession::NEONSession(Context & c, const Uri & uri, const RequestParams & p, DavixError** err) :
    _f(ContextExplorer::SessionFactoryFromContext(c)),
    _sess(NULL),
    _params(p),
    _last_error(NULL),
    _session_recycling(true)
{
        _f.createNeonSession(uri, &_sess, err);
        if(_sess)
            configureSession(_sess, p, &NEONSession::provide_login_passwd_fn, this, &NEONSession::provide_clicert_fn, this);
}


NEONSession::NEONSession(NEONSessionFactory & f, const Uri & uri, const RequestParams & p, DavixError** err) :
    _f(f),
    _sess(NULL),
    _params(p),
    _last_error(NULL),
    _session_recycling(true)
{
    _f.createNeonSession(uri, &_sess, err);
    if(_sess)
        configureSession(_sess, p, &NEONSession::provide_login_passwd_fn, this, &NEONSession::provide_clicert_fn, this);
}


NEONSession::~NEONSession(){
#   ifndef _DISABLE_SESSION_REUSE
        if(_sess){
            if(_session_recycling)
                _f.storeNeonSession(_sess, NULL);
            else
                ne_session_destroy(_sess);
        }
#   endif

}


void configureSession(ne_session *_sess, const RequestParams &params, ne_auth_creds lp_callback, void* lp_userdata,
                      ne_ssl_provide_fn cred_callback,  void* cred_userdata){
    if(strcmp(ne_get_scheme(_sess), "https") ==0) // fix a libneon bug with non ssl connexion
        ne_ssl_trust_default_ca(_sess);

    // register redirection management
    ne_redirect_register(_sess);

    // define user agent
    ne_set_useragent(_sess, params.getUserAgent().c_str());

    if(params.getSSLCACheck() == false){ // configure ssl check
        DAVIX_DEBUG("NEONRequest : disable ssl verification");
        ne_ssl_set_verify(_sess, validate_all_certificate, NULL);
    }

    // if authentification for login/password
    if( params.getClientLoginPassword().first.empty() == false
            || params.getClientLoginPasswordCallback().first != NULL){
        DAVIX_DEBUG("NEONSession : enable login/password authentication");
        ne_set_server_auth(_sess, lp_callback, lp_userdata);
    }else{
        DAVIX_DEBUG("NEONSession : disable login/password authentication");
    }

    // if authentification for cli cert by callback
    if( params.getClientCertCallbackX509().first != NULL){
        DAVIX_DEBUG("NEONSession : enable client cert authentication by callback ");
        ne_ssl_provide_clicert(_sess, cred_callback, cred_userdata);
    }else if( params.getClientCertX509().hasCert()){
        ne_ssl_set_clicert(_sess, X509CredentialExtra::extract_ne_ssl_clicert(params.getClientCertX509()));
        DAVIX_DEBUG("NEONSession : enable client cert authentication with predefined cert");
    }else{
          DAVIX_DEBUG("NEONSession : disable client cert authentication");
    }

    if( timespec_isset(params.getOperationTimeout())){
        DAVIX_DEBUG("NEONSession : define operation timeout to %d", params.getOperationTimeout());
        ne_set_read_timeout(_sess, (int) params.getOperationTimeout()->tv_sec);
    }
    if(timespec_isset(params.getConnectionTimeout())){
        DAVIX_DEBUG("NEONSession : define connection timeout to %d", params.getConnectionTimeout());
#ifndef _NEON_VERSION_0_25
        ne_set_connect_timeout(_sess, (int) params.getConnectionTimeout()->tv_sec);
#endif
    }

    for(std::vector<std::string>::const_iterator it = params.listCertificateAuthorityPath().begin(); it < params.listCertificateAuthorityPath().end(); it++){
        struct stat st;
        if ( stat(it->c_str(), &st) < 0 || S_ISDIR(st.st_mode) == false){
            DAVIX_LOG(DAVIX_LOG_WARNING, "NEONSession : CA Path invalid : %s, %s ", it->c_str(), (errno != 0)?strerror(errno):strerror(ENOTDIR));
            errno = 0;
        }else{
            DAVIX_TRACE(" add CA PATH %s", it->c_str());
            ne_ssl_truse_add_ca_path(_sess, it->c_str());
        }
    }

    ne_set_session_flag(_sess, NE_SESSFLAG_PERSIST, params.getKeepAlive());

}





}
