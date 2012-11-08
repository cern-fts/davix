#include "neonrequest.hpp"

#include <cstring>
#include <libs/time_utils.h>
#include <ne_redirect.h>

namespace Davix {

/*
  authentification handle for neon
 */


void neon_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err){
    StatusCode::Code code;
    std::string str;
    switch(ne_status){
        case NE_OK:
            code = StatusCode::OK;
            str= "Status Ok";
            break;
        case NE_ERROR:
             str = std::string("Neon error : ").append(ne_get_error(sess));
             code = StatusCode::ConnexionProblem;
             break;
        case NE_LOOKUP:
             code = StatusCode::NameResolutionFailure;
             str= "Domain Name resolution failed";
             break;
        case NE_AUTH:
            code = StatusCode::authentificationError;
            str=  "Authentification Failed on server";
            break;
        case NE_PROXYAUTH:
            code = StatusCode::authentificationError;
            str=  "Authentification Failed on proxy";
        case NE_CONNECT:
            code = StatusCode::ConnexionProblem;
            str= "Could not connect to server";
            break;
        case NE_TIMEOUT:
            code = StatusCode::ConnexionTimeout;
            str= "Connection timed out";
        case NE_FAILED:
            code = StatusCode::SessionCreationError;
            str=  "The precondition failed";
            break;
        case NE_RETRY:
            code = StatusCode::RedirectionNeeded;
            str= "Retry Request";
            break;
        default:
            code= StatusCode::UnknowError;
            str= "Unknow Error from libneon";
    }
    DavixError::setupError(err,scope, code, str);
}


static int validate_all_certificate(void *userdata, int failures,
                                const ne_ssl_certificate *cert){
    return 0;
}



void NEONRequest::provide_clicert_fn(void *userdata, ne_session *sess,
                                         const ne_ssl_dname *const *dnames,
                                         int dncount){

    NEONRequest* req = static_cast<NEONRequest*>(userdata);
    DavixError* tmp_err=NULL;
    davix_auth_info_t auth_info;
    auth_info.auth = DAVIX_CLI_CERT_PKCS12;
    davix_auth_callback auth_call = req->params.getAuthentificationCallbackFunction();

    davix_log_debug("NEONRequest > clicert callback ");
    if( auth_call == NULL){
        davix_log_debug("NEONRequest : No callback specified, cancel authentification");
        return;
    }else{
        davix_log_debug("NEONRequest > call authentification callback ");
        int ret = auth_call((davix_auth_t) req, &auth_info, req->params.getAuthentificationCallbackData(), (davix_error_t*) &tmp_err); // try to get authentification
        davix_log_debug("NEONRequest > return from authentification callback ");
        if(ret !=0 && tmp_err == NULL){
            DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::authentificationError, "Authentification callback returned with CANCEL without DavixError object");
        }
    }
    if(tmp_err){
       DavixError::propagateError(&(req->auth_last_error), tmp_err);
    }
    return;
}

int NEONRequest::provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password){
    NEONRequest * req = static_cast<NEONRequest*>(userdata);

     davix_log_debug("NEONRequest > Try to get auth/password authentification ");
     davix_auth_info_t auth_info;
    // memset(&auth_info,0,sizeof(davix_auth_info_t));
     davix_auth_callback auth_call = req->params.getAuthentificationCallbackFunction();
     auth_info.auth = DAVIX_LOGIN_PASSWORD;
     DavixError* tmp_err=NULL;

     if(auth_call  == NULL){
         davix_log_debug("NEONRequest : No credential specified, cancel login/password authentification");
         return -1;
     }

     davix_log_debug("NEONRequest > call authentification callback ");
     int ret = auth_call((davix_auth_t) req, &auth_info, req->params.getAuthentificationCallbackData(), (davix_error_t*) &tmp_err); // try to get authentification
     davix_log_debug("NEONRequest > return from authentification callback ");
     if(ret != 0){
            DavixError::propagateError(&(req->auth_last_error), tmp_err);
            return -2;
     }

     if( req->_passwd.empty()
        || req->_login.empty() ){
        davix_log_debug("NEONRequest > Login/Password missings ....");
        return -1;
    }
    davix_log_debug("NEONRequest > setup authentification pwd/login....");
    g_strlcpy(username, req->_login.c_str(), NE_ABUFSIZ);
    g_strlcpy(password, req->_passwd.c_str(), NE_ABUFSIZ);
    req->_login.clear();
    req->_passwd.clear();
    return 0;

}




NEONRequest::NEONRequest(NEONSessionFactory* f, ne_session * sess, const Uri & uri_req) : _request_type("GET")
{
    _sess=sess;
    _login.clear();
    _passwd.clear();
    _current = _orig = uri_req;
    _req=NULL;
    _f = f;
    req_started= req_running =false;
    auth_last_error = NULL;
    _content_ptr = NULL;
    _content_len =0;
}

NEONRequest::~NEONRequest(){
    // safe destruction of the request
    if(req_running) endRequest(NULL);

    free_request();
    //ne_forget_auth(_sess);
#ifndef _DISABLE_SESSION_REUSE
    _f->internal_release_session_handle(_sess);
#endif
}

int NEONRequest::create_req(DavixError** err){
    if(_req != NULL || req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::alreadyRunning, "Http request already started, Error");
        return -1;
    }

    configure_sess();
    _req= ne_request_create(_sess, _request_type.c_str(), _current.getPathAndQuery().c_str());
    configure_req();

    return 0;
}

void NEONRequest::configure_sess(){
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
        ne_ssl_provide_clicert(_sess, &NEONRequest::provide_clicert_fn, this);
        ne_set_server_auth(_sess, &NEONRequest::provide_login_passwd_fn, this);
    }



    if( timespec_isset(params.getOperationTimeout())){
        davix_log_debug("NEONRequest : define operation timeout to %d", params.getOperationTimeout());
        ne_set_read_timeout(_sess, (int) params.getOperationTimeout()->tv_sec);
    }
    if(timespec_isset(params.getConnexionTimeout())){
        davix_log_debug("NEONRequest : define connexion timeout to %d", params.getConnexionTimeout());
#ifndef _NEON_VERSION_0_25
        ne_set_connect_timeout(_sess, (int) params.getConnexionTimeout()->tv_sec);
#endif
    }
}

void NEONRequest::configure_req(){
    for(size_t i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }

    if(_content_ptr && _content_len >0){
        ne_set_request_body_buffer(_req, _content_ptr, _content_len);
    }
}

int NEONRequest::negotiate_request(DavixError** err){

    const int n_limit = 10;
    int code, status, end_status = NE_RETRY;
    int n =0;

    davix_log_debug(" ->   Davix negociate request ... ");
    if(req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::alreadyRunning, "Http request already started, Error");
        davix_log_debug(" Davix negociate request ... <-");
        return -1;
    }

    req_started = req_running= true;

    while(end_status == NE_RETRY && n < n_limit){
        davix_log_debug(" ->   NEON start internal request ... ");

        if( (status = ne_begin_request(_req)) != NE_OK && status != NE_REDIRECT){
            if( status == NE_ERROR && strstr(ne_get_error(_sess), "Could not") != NULL ){ // bugfix against neon keepalive problem
               davix_log_debug("Connexion close, retry...");
               n++;
               continue;
            }

            req_started= req_running == false;
            neon_to_davix_code(status, _sess, davix_scope_http_request(),err);
            davix_log_debug(" Davix negociate request ... <-");
            return -1;
        }

        code = getRequestCode();
        switch(code){
            case 301:
            case 302:
            case 307:
                if( end_status != NE_OK
                        && end_status != NE_RETRY
                        && end_status != NE_REDIRECT){
                    req_started= req_running = false;
                    neon_to_davix_code(status, _sess, davix_scope_http_request(),err);
                    davix_log_debug(" Davix negociate request ... <-");
                    return -1;
                }
                ne_discard_response(_req);              // Get a valid redirection, drop request content
                end_status = ne_end_request(_req);      // submit the redirection
                if(redirect_request(err) <0){           // accept redirection
                    davix_log_debug(" Davix negociate request ... <-");
                    return -1;
                }
                end_status = NE_RETRY;
                break;
            case 401: // authentification requested, do retry
                ne_discard_response(_req);
                end_status = ne_end_request(_req);
                if( end_status != NE_RETRY){
                    req_started= req_running = false;
                    neon_to_davix_code(status, _sess, davix_scope_http_request(),err);
                    return -1;
                }
                davix_log_debug(" ->   NEON receive %d code, %d .... request again ... ", code, end_status);
                break;
            default:
                end_status = 0;
                break;

        }
        n++;
    }

    if(n >= n_limit){
        DavixError::setupError(err,davix_scope_http_request(),StatusCode::authentificationError, "overpass the maximum number of authentification try, cancel");
        return -2;
    }
    davix_log_debug(" Davix negociate request ... <-");
    return 0;
}

int NEONRequest::redirect_request(DavixError **err){
    const ne_uri * new_uri = ne_redirect_location(_sess);
    if(!new_uri){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "Impossible to get the new redirected destination");
        return -1;
    }


    davix_log_debug("redirection from %s://%s/%s to %s", ne_get_scheme(_sess),
                      ne_get_server_hostport(_sess), _current.getPathAndQuery().c_str(), ne_uri_unparse(new_uri));

    // setup new path & session target
    _current= Uri(ne_uri_unparse(new_uri));

    // recycle old request and session
    if( _f->storeNeonSession(_sess, err) <0){
        return -1;
    }
    free_request();
    _sess = NULL;

    // renew request
    req_started = false;
    // create a new couple of session + request
    if( _f->createNeonSession(_current, &_sess, err) <0 ){
        return -1;
    }
    if( create_req(err) <0){
        return -1;
    }
    req_started= true;
    return 0;
}

int NEONRequest::executeRequest(DavixError** err){
    ssize_t read_status=1;
    DavixError* tmp_err=NULL;

    if( create_req(&tmp_err) < 0){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    davix_log_debug(" -> NEON start synchronous  request... ");
    if( negotiate_request(&tmp_err) < 0){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

    while(read_status > 0){
        davix_log_debug(" -> NEON Read data flow... ");
        size_t s = _vec.size();
        _vec.resize(s + NEON_BUFFER_SIZE);
        read_status= ne_read_response_block(_req, &(_vec[s]), NEON_BUFFER_SIZE );
        if( read_status >= 0 && read_status != NEON_BUFFER_SIZE){
           _vec.resize(s +  read_status);
        }

    }
    // push a last NULL char for safety
    _vec.push_back('\0');

    if(read_status < 0){
        neon_to_davix_code(read_status, _sess, davix_scope_http_request(), &tmp_err);
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

   if( endRequest(&tmp_err) < 0){
       DavixError::propagateError(err, tmp_err);
       return -1;
   }

    davix_log_debug(" -> End synchronous request ... ");
    return 0;
}

int NEONRequest::beginRequest(DavixError** err){
    DavixError* tmp_err=NULL;
    int ret = -1;
    ret= create_req(&tmp_err);


    if( ret >=0){
       ret= negotiate_request(&tmp_err);
    }

    if(ret <0)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

ssize_t NEONRequest::readBlock(char* buffer, size_t max_size, DavixError** err){
    ssize_t read_status=-1;

    if(_req == NULL){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::alreadyRunning, "No request started");
        return -1;
    }


    read_status= ne_read_response_block(_req, buffer, max_size );
    if(read_status <0){
       DavixError::setupError(err, davix_scope_http_request(), StatusCode::ConnexionProblem, "Invalid Read in request");
       return -1;
    }
    return read_status;
}

int NEONRequest::endRequest(DavixError** err){
    int status;

    if(_req == NULL || req_running == false){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::alreadyRunning, "Http request already started, Error");
        return -1;
    }

    req_running = false;
    if( (status = ne_end_request(_req)) != NE_OK){
        DavixError* tmp_err=NULL;
        neon_to_davix_code(status, _sess, davix_scope_http_request(), &tmp_err);
        if(tmp_err)
            davix_log_debug("NEONRequest::endRequest -> error %d Error closing request -> %s ", tmp_err->getStatus(), tmp_err->getErrMsg().c_str());
        DavixError::clearError(&tmp_err);
    }
    return 0;
}

void NEONRequest::clear_result(){
    _vec.clear();
}



int NEONRequest::getRequestCode(){
    return ne_get_status(_req)->code;
}

const std::vector<char> & NEONRequest::getAnswerContent(){
    return _vec;
}

bool NEONRequest::getAnswerHeader(const std::string &header_name, std::string &value){
    const char* answer_content = ne_get_response_header(_req, header_name.c_str());
    if(answer_content){
        value = answer_content;
        return true;
    }
    return false;
}

int NEONRequest::do_pkcs12_cert_authentification(const char *filename_pkcs12, const char *passwd, DavixError** err){
    int ret;
    ne_ssl_client_cert * cert = ne_ssl_clicert_read(filename_pkcs12);
    if(cert == NULL){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::credentialNotFound, "impossible to load credential pkcs12");
        return -1;
    }

    // try to decrypt
    int crypt_state = ne_ssl_clicert_encrypted(cert);
    if(crypt_state ==0 ){
        davix_log_debug("NEONRequest : Credential unencrypted, try to use it directly");
    }else{
        davix_log_debug("NEONRequest : Credential is encrypted, try to decrypt credential");

        if(passwd == NULL){
            DavixError::setupError(err, davix_scope_http_request(), StatusCode::loginPasswordError, "no password provided and credential encrypted");
            return -1;
        }

        if ((ret= ne_ssl_clicert_decrypt(cert, passwd)) != 0){
            DavixError::setupError(err, davix_scope_http_request(), StatusCode::loginPasswordError, "Unable to decrypt credential, bad password");
            return -1;
        }
    }
    ne_ssl_set_clicert(_sess, cert);
    ne_ssl_clicert_free(cert);
    davix_log_debug("NEONRequest : associate credential to the current session");
    return 0;
}

int NEONRequest::do_login_passwd_authentification(const char *login, const char *passwd, DavixError** err){
   this->_login = (char*) login;
   this->_passwd = (char*)  passwd;
   return 0;
}


void NEONRequest::setRequestBodyString(const std::string & body){
    davix_log_debug("NEONRequest : add request content of size %s ", body.c_str());
    _content_body = std::string(body);
    _content_ptr = (char*) _content_body.c_str();
    _content_len = strlen(_content_ptr);
}

void NEONRequest::setRequestBodyBuffer(const void *buffer, size_t len){
    _content_ptr = (char*) buffer;
    _content_len = len;
}


void NEONRequest::free_request(){
    if(_req != NULL){
        ne_request_destroy(_req);
        _req=NULL;
    }
}

} // namespace Davix
