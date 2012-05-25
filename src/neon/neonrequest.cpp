#include "neonrequest.h"

#include <glibmm/error.h>
#include <glibmm/quark.h>
#include <cstring>

namespace Davix {


std::string  translate_neon_status(int ne_status, ne_session* sess, int* errno_code){
    switch(ne_status){
        case NE_OK:
            *errno_code =0;
            return "Success, no Error";
        case NE_ERROR:
             *errno_code = ECOMM;
             return ne_get_error(sess);
        case NE_LOOKUP:
             *errno_code = ENOSYS;
             return "Domain Name resolution failed";
        case NE_AUTH:
            *errno_code = EPERM;
            return "Authentification Failed on server";
        case NE_PROXYAUTH:
            *errno_code = EPERM;
            return "Authentification Failed on proxy";
        case NE_CONNECT:
            *errno_code = ECOMM;
            return "Could not connect to server";
        case NE_TIMEOUT:
            *errno_code = ETIME;
            return "Connection timed out";
        case NE_FAILED:
            *errno_code = EINVAL;
            return "The precondition failed";
        case NE_RETRY:
            *errno_code = EAGAIN;
            return "Retry Request";
        default:
            *errno_code = ECOMM;
            return "Unknow Error from libneon";
    }
}


static int validate_all_certificate(void *userdata, int failures,
                                const ne_ssl_certificate *cert){
    return 0;
}

int NEONRequest::try_pkcs12_authentification(ne_session *sess, const ne_ssl_dname *const *dnames){
    davix_log_debug("NEONRequest : Try to decrypt credential ");
    davix_auth_info_t auth_info;
    memset(&auth_info,0,sizeof(davix_auth_info_t));
    GError* tmp_err=NULL;
   davix_log_debug("NEONRequest > call authentification callback ");

    int ret = _call(this, &auth_info, _user_auth_callback_data, &tmp_err); // try to get authentification
    davix_log_debug("NEONRequest > return from authentification callback ");
    if(ret != 0)
            throw Glib::Error(tmp_err);
    return 0;
}

void NEONRequest::add_full_request_content(const std::string & body){
    davix_log_debug("NEONRequest : add request content of size %s ", body.c_str());
    _content_body = std::string(body);
}

void NEONRequest::provide_clicert_fn(void *userdata, ne_session *sess,
                                         const ne_ssl_dname *const *dnames,
                                         int dncount){

    NEONRequest* req = (NEONRequest*) userdata;
    davix_log_debug("NEONRequest > clicert callback ");
    if( req->_call == NULL){
        davix_log_debug("NEONRequest : No credential specified, cancel authentification");
        return;
    }else{
        req->try_pkcs12_authentification(sess, dnames);

    }
}


NEONRequest::NEONRequest(NEONSessionFactory* f, ne_session * sess, RequestType typ, const std::string & path,
                         void * user_auth_callback_data,
                         davix_auth_callback call) : _request_type("GET")
{
    _sess=sess;
    _path = path;
    _req=NULL;
    _call = call;
    _f = f;
    _user_auth_callback_data = user_auth_callback_data;
    ne_ssl_provide_clicert(sess, &NEONRequest::provide_clicert_fn, this);
}

NEONRequest::~NEONRequest(){
    finish_block();
    _f->internal_release_session_handle(_sess);
}

void NEONRequest::create_req(){
    if(_req != NULL)
      finish_block();
    _req= ne_request_create(_sess, _request_type.c_str(), _path.c_str());

    for(size_t i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }
    if(_content_body.size() > 0)
        ne_set_request_body_buffer(_req, _content_body.c_str(), _content_body.size());
}

void NEONRequest::set_requestcustom(const std::string &request_str){
    _request_type = request_str;
}

void NEONRequest::add_header_field(const std::string &field, const std::string &value){
    _headers_field.push_back(std::pair<std::string, std::string> (field, value));
}

int NEONRequest::execute_sync(){
    int status;
    int err_code;
    ssize_t read_status=1;

    create_req();
    davix_log_debug(" -> NEON start synchronous  request... ");
    if( (status = ne_begin_request(_req)) != NE_OK){
        std::string err_str = translate_neon_status(status, _sess,&err_code);
        throw Glib::Error(Glib::Quark("NEONRequest::Execute_sync"), err_code, std::string("begin start : ").append(err_str));
    }

    while(read_status > 0){
        davix_log_debug(" -> NEON Read data flow... ");
        size_t s = _vec.size();
        _vec.resize(s + NEON_BUFFER_SIZE);
        read_status= ne_read_response_block(_req, &(_vec[s]), NEON_BUFFER_SIZE );
        if( read_status > 0 && read_status != NEON_BUFFER_SIZE){
           _vec.resize(s +  read_status);
        }

    }

    if(read_status < 0){
        std::string err_str = translate_neon_status(read_status, _sess,&err_code);
        throw Glib::Error(Glib::Quark("NEONRequest::Execute_sync"), err_code, std::string(" NEON reading error : ").append(err_str));
    }
    davix_log_debug(" -> End synchronous request ... ");
    return 0;
}

void NEONRequest::execute_block(){
    int status;
    int err_code;

    create_req();

    if( (status = ne_begin_request(_req)) != NE_OK){
        std::string err_str = translate_neon_status(status, _sess,&err_code);
        throw Glib::Error(Glib::Quark("NEONRequest::Execute_sync"), err_code, err_str);
    }
}

ssize_t NEONRequest::read_block(char* buffer, size_t max_size){
    ssize_t read_status=1;

    if(_req == NULL)
        throw Glib::Error(Glib::Quark("NEONRequest::read_block"), EINVAL, "No request started");


    read_status= ne_read_response_block(_req, buffer, max_size );
    if(read_status <0){
       throw Glib::Error(Glib::Quark("NEONRequest::read_block"), EIO, "Invalid Read in request");
    }
    return read_status;
}

void NEONRequest::finish_block(){
    int status;
    int err_code;

    if(_req != NULL){
        if( (status = ne_end_request(_req)) != NE_OK){
            std::string err_str = translate_neon_status(status, _sess,&err_code);
            throw Glib::Error(Glib::Quark("NEONRequest::finish_block"), err_code, std::string("Error closing request ->").append(err_str));
        }
        ne_request_destroy(_req);
        _req=NULL;
    }
}

void NEONRequest::clear_result(){
    _vec.clear();
}

void NEONRequest::disable_ssl_ca_check(){
     davix_log_debug("NEONRequest : disable ssl verification");
     ne_ssl_set_verify(_sess, validate_all_certificate, NULL);
}

int NEONRequest::get_request_code(){
    if(_req == NULL)
        throw Glib::Error(Glib::Quark("NEONRequest::get_request_code"), EINVAL, "No request started and try to get req code ..");
    return ne_get_status(_req)->code;
}

const std::vector<char> & NEONRequest::get_result(){
    return _vec;
}

void NEONRequest::try_set_pkcs12_cert(const char *filename_pkcs12, const char *passwd){
    int ret;
    ne_ssl_client_cert * cert = ne_ssl_clicert_read(filename_pkcs12);
    if(cert == NULL)
        throw Glib::Error(Glib::Quark("NEONRequest::try_set_pkcs12_cert"), EINVAL, "Unable to load credential " + std::string(filename_pkcs12) + ", failure");
    // try to decrypt
    int crypt_state = ne_ssl_clicert_encrypted(cert);
    if(crypt_state ==0 ){
        davix_log_debug("NEONRequest : Credential unencrypted, use it directly");
    }else{
        davix_log_debug("NEONRequest : Credential is encrypted, try to decrypt credential");

        if(passwd == NULL)
           throw Glib::Error(Glib::Quark("NEONRequest::try_set_pkcs12_cert"), DAVIX_ERROR_NOPASSWD, " No password provided for encrypted credential ");
        if ((ret= ne_ssl_clicert_decrypt(cert, passwd)) != 0)
            throw Glib::Error(Glib::Quark("NEONRequest::try_set_pkcs12_cert"), DAVIX_ERROR_BADPASSWD, " Unable to decrypt credential bad password ");
    }
    ne_ssl_set_clicert(_sess, cert);
    ne_ssl_clicert_free(cert);
    davix_log_debug("NEONRequest : associate credential to the current session");

}

} // namespace Davix
