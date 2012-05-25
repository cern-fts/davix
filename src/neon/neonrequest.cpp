#include "neonrequest.h"

#include <glibmm/error.h>
#include <glibmm/quark.h>

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


NEONRequest::NEONRequest(ne_session * sess, RequestType typ, const std::string & path,
                         void * user_auth_callback_data,
                         davix_auth_callback call) : _request_type("GET")
{
    _sess=sess;
    _path = path;
    _req=NULL;
    _call = call;
    _user_auth_callback_data = user_auth_callback_data;
}

NEONRequest::~NEONRequest(){
    finish_block();
    ne_session_destroy(_sess);

}

void NEONRequest::create_req(){
    if(_req != NULL)
      finish_block();
    _req= ne_request_create(_sess, _request_type.c_str(), _path.c_str());

    for(int i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }
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
            throw Glib::Error(Glib::Quark("NEONRequest::finish_block"), err_code, std::string("Error closing request").append(err_str));
        }
        ne_request_destroy(_req);
        _req=NULL;
    }
}

void NEONRequest::clear_result(){
    _vec.clear();
}

void NEONRequest::disable_ssl_ca_check(){

}

int NEONRequest::get_request_code(){
    if(_req == NULL)
        throw Glib::Error(Glib::Quark("NEONRequest::get_request_code"), EINVAL, "No request started and try to get req code ..");
    return ne_get_status(_req)->code;
}

const std::vector<char> & NEONRequest::get_result(){
    return _vec;
}

} // namespace Davix
