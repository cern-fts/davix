/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
#include "neonsession.hpp"
#include <ne_redirect.h>
#include <libs/time_utils.h>
#include <auth/davixx509cred_internal.hpp>
#include <neon/neonsessionfactory.hpp>
#include <utils/stringutils.hpp>

const char* davix_neon_key="davix_key";

using namespace StrUtil;

namespace Davix{


static std::pair<std::string, std::string> userInfo_to_auth(const std::string & userInfo){
    std::pair<std::string , std::string > id;
    std::string::const_iterator it = std::find(userInfo.begin(), userInfo.end(), ':');
    id.first = std::string(userInfo.begin(), it);
    it = ((it != userInfo.end())?(it+1):(it));
    id.second = std::string(it, userInfo.end());
    return id;
}


static int validate_all_certificate(void *userdata, int failures,
                                const ne_ssl_certificate *cert){
    (void) userdata;
    (void) failures;
    (void) cert;
    return 0;
}

const int n_max_auth = 20;


ne_session* NEONSession::get_ne_sess() {
  return _sess->session;
}

void NEONSession::authNeonCliCertMapper(void *userdata, ne_session *sess,
                                         const ne_ssl_dname *const *dnames,
                                         int dncount){
    (void) dncount;
    (void) sess;
    (void) dnames;
    NEONSession* req = static_cast<NEONSession*>(userdata);

    X509Credential cert;
    const RequestParams* params = &req->_params;
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "clicert callback ");
    DavixError::clearError(&(req->_last_error));

    if(params->getClientCertFunctionX509()){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "call client cert callback ");
        SessionInfo infos;
        TRY_DAVIX{
            params->getClientCertFunctionX509()(infos, cert);
            if(cert.hasCert() == false){
                throw DavixException(davix_scope_x509cred(), StatusCode::AuthenticationError,
                                     "No valid credential given ");
            }
            ne_ssl_set_clicert(req->_sess->session, X509CredentialExtra::extract_ne_ssl_clicert(cert));
        }CATCH_DAVIX(&(req->_last_error));
    }
    return;
}

int NEONSession::provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password){
    (void) realm;
    NEONSession * req = static_cast<NEONSession*>(userdata);
    DavixError * tmp_err=NULL;
    int ret =-1;


    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Try to get auth/password authentication from client");

     if(attempt > n_max_auth ){
         DavixError::setupError(&(req->_last_error), davix_scope_http_request(), StatusCode::LoginPasswordError,
                                "Overpass allowed number of authentication attempt");
     }

     const std::pair<authCallbackLoginPasswordBasic, void*> retcallback(req->_params.getClientLoginPasswordCallback());
     std::pair<std::string , std::string > id;

     if(req->_u.getUserInfo().size() > 0){
        id = userInfo_to_auth(req->_u.getUserInfo());
     }else{
        id = req->_params.getClientLoginPassword();
     }

     if(id.first.empty() == false){
              copy_std_string_to_buff(username, NE_ABUFSIZ, id.first.c_str());
              copy_std_string_to_buff(password, NE_ABUFSIZ, id.second.c_str());
     }else if(retcallback.first != NULL){
         DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Try callback for login/passwd for {} time", attempt+1);
         SessionInfo infos;
         std::string tmp_login, tmp_password;
         DavixError::clearError(&(req->_last_error));
         if( (ret = retcallback.first(retcallback.second, infos, tmp_login, tmp_password, attempt, &tmp_err) ) <0){
             if(!tmp_err)
                 DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::LoginPasswordError,
                                        "No valid login/passwd");
              DavixError::propagateError(&(req->_last_error), tmp_err);
              return -1;
         }
         copy_std_string_to_buff(username, NE_ABUFSIZ, tmp_login.c_str());
         copy_std_string_to_buff(password, NE_ABUFSIZ, tmp_password.c_str());
     }

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "get login/password with success...try server submission ");
    return 0;

}

NEONSession::NEONSession(NEONSessionFactory &f, NeonHandlePtr sess, const Uri & uri, const RequestParams & p, DavixError** err) :
    _f(f),
    _sess(std::move(sess)),
    _params(p),
    _last_error(NULL),
    _session_recycling(_f.getSessionCaching() && p.getKeepAlive()),
    reused(false),
    _u(uri)
{
        if(_sess)
            configureSession(_sess, _u, p, &NEONSession::provide_login_passwd_fn, this, &NEONSession::authNeonCliCertMapper, this, reused);
}

NEONSession::~NEONSession(){
        if(_sess){
            if(_session_recycling) {
                _f.storeNeonSession(std::move(_sess));
            }
            else {
                _sess.reset();
            }
        }
        DavixError::clearError(&_last_error);
}


void configureSession(NeonHandlePtr &_sess, const Uri & _u, const RequestParams &params, ne_auth_creds lp_callback, void* lp_userdata,
                      ne_ssl_provide_fn cred_callback,  void* cred_userdata, bool & reused){

    void* state = ne_get_session_private(_sess->session, davix_neon_key);
    if(state != NULL){
        reused = true;
    }

    if(state == NULL || state != params.getParmState()){
        // no configuration done, need to configure
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "configure session...");

        if(strcmp(ne_get_scheme(_sess->session), "https") ==0) // fix a libneon bug with non ssl connexion
            ne_ssl_trust_default_ca(_sess->session);

        // register redirection management
        ne_redirect_register(_sess->session);

        // define user agent
        ne_set_useragent(_sess->session, params.getUserAgent().c_str());

        if(params.getSSLCACheck() == false){ // configure ssl check
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "disable ssl verification");
            ne_ssl_set_verify(_sess->session, validate_all_certificate, NULL);
        }

        if(timespec_isset(params.getConnectionTimeout())){
            const int timeout = static_cast<int>(params.getConnectionTimeout()->tv_sec);
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "define connection timeout to {}" , timeout);
            ne_set_connect_timeout(_sess->session, timeout);
        }

        if(timespec_isset(params.getOperationTimeout())){
            const int timeout = static_cast<int>(params.getOperationTimeout()->tv_sec);
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "define operation timeout to {}" , timeout);
            ne_set_read_timeout(_sess->session, timeout);
        }

        for(std::vector<std::string>::const_iterator it = params.listCertificateAuthorityPath().begin(); it < params.listCertificateAuthorityPath().end(); it++){
            struct stat st;
            if ( stat(it->c_str(), &st) < 0 || S_ISDIR(st.st_mode) == false){
                DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HTTP, "CA Path invalid : {}, {} ", *it, ((errno != 0)?strerror(errno):strerror(ENOTDIR)));
                errno = 0;
            }else{
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "add CA PATH {}", *it);
                ne_ssl_truse_add_ca_path(_sess->session, it->c_str());
            }
        }

        ne_set_session_flag(_sess->session, NE_SESSFLAG_PERSIST, params.getKeepAlive());

        // setup sess key
        ne_set_session_private(_sess->session, davix_neon_key, params.getParmState());
    }
    // configure callback for new request
    if( params.getClientLoginPassword().first.empty() == false
            || _u.getUserInfo().size() > 0
            || params.getClientLoginPasswordCallback().first != NULL){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "enable login/password authentication");
        ne_set_server_auth(_sess->session, lp_callback, lp_userdata);
    }else{
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "disable login/password authentication");
    }

    // if authentication for cli cert by callback
    if( params.getClientCertFunctionX509()){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "enable client cert authentication by callback ");
        ne_ssl_provide_clicert(_sess->session, cred_callback, cred_userdata);
    }else{
          DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "disable client cert authentication");
    }


}





}
