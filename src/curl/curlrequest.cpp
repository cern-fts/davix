#include "curlrequest.h"
#include <string>
#include <cstring>
#include <errno.h>
#include <glibmm/error.h>
#include <glibmm/quark.h>

namespace Davix {


CURLRequest::CURLRequest(CURL* handle, RequestType typ, const std::string & url) : _url(url), _typ(typ), _handle(handle)
{
    _init(handle, typ, url);
    _call = NULL;
    _userdata = NULL;
}

CURLRequest::CURLRequest(CURL *handle, RequestType typ, const std::string &url, davix_auth_callback call, void *userdata){
    _init(handle, typ, url);
    _call = call;
    _userdata = userdata;
}

void CURLRequest::_init(CURL *handle, RequestType typ, const std::string &url){
    _handle = handle;
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str()); // setup url to contact
    buff_err = new char[CURL_ERROR_SIZE+1];
    memset(buff_err, '\0',(CURL_ERROR_SIZE+1)*sizeof(char) );
    curl_easy_setopt(handle,CURLOPT_ERRORBUFFER, buff_err); //enable verbose error string
    curl_easy_setopt(handle,CURLOPT_FOLLOWLOCATION,TRUE); // eable redirection support
    headers_fields = NULL; // reset the overload for the header fields
}

CURLRequest::~CURLRequest(){
    finish_block();
    curl_easy_cleanup(_handle);
    curl_slist_free_all(headers_fields);
    delete[] buff_err;
}

/**
  Write callback for CURL, One-time-read
 */
size_t CURLRequest::write_data_curl(char *ptr, size_t size, size_t nmemb, void *userdata){
    const size_t n_bytes = size * nmemb;
    std::vector<char>* vec = (std::vector<char>*) userdata;


    for(size_t i = 0; i < n_bytes; ++i){
        vec->push_back(ptr[i]);
    }
    return n_bytes;
}

size_t CURLRequest::write_data_curl_chunk_new(char *ptr, size_t size, size_t nmemb, void *userdata){
    CURLTransferState* req = (CURLTransferState*) userdata;
   const size_t n_bytes = size * nmemb;
    if(req->_abort)
        return n_bytes+2; // return inconsistant value : want to throw an eror


    if(req->internal_buffer.size() > req->_max_size_chunk){ // buffer full
        req->_pause = true;
        return CURL_WRITEFUNC_PAUSE;
    }

    for(size_t i = 0; i < n_bytes; ++i){
        req->internal_buffer.push_back(ptr[i]);
    }
    req->_pause = false;
    return n_bytes;
}

void CURLRequest::add_header_field(const std::string &field, const std::string &value){
    std::string field_line(field);
    field_line.append(": ");
    field_line.append(value);
    headers_fields = curl_slist_append(headers_fields, field_line.c_str());
}

int CURLRequest::execute_sync(){

    vec.clear();
    CURLcode r;
    curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, &write_data_curl);
    curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &vec);
    curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, headers_fields);
    get_credential();

    if(( r = curl_easy_perform(_handle) ) != CURLE_OK){
        std::ostringstream s;
        s << " Execute request err async start: " << curl_easy_strerror(r) << ", " << std::string(buff_err) << std::endl;
        throw Glib::Error( Glib::Quark("CURLRequest::execute_chunk::CURL_ERROR"),  r,s.str() );
    }
    return 0;
}

void CURLRequest::execute_block(size_t buff_size){
    if(state._started)
        throw Glib::Error( Glib::Quark("CURLRequest::execute_block::start"), EINVAL, "transfer request started");

    CURLcode r;
    state.clear();
    curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, &write_data_curl_chunk_new);
    curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &state);
    curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, headers_fields);
    get_credential();
    state._started=true;
    state._max_size_chunk = buff_size;

    if(( r = curl_easy_perform(_handle) ) != CURLE_OK){
        std::ostringstream s;
        s << " Execute request err async start: " << curl_easy_strerror(r) << ", " << std::string(buff_err) << std::endl;
        throw Glib::Error( Glib::Quark("CURLRequest::execute_chunk::CURL_ERROR"),  r,s.str() );
    }

}


void CURLRequest::read_block(std::vector<char> &buffer, size_t max_size){
    if( state._started == false)
          throw Glib::Error( Glib::Quark("CURLRequest::execute_block::start"), EINVAL, "transfer request not started");

    CURLcode r;
    if( state.internal_buffer.size() < max_size && state._pause == true ){
        if(( r = curl_easy_pause(_handle, CURLPAUSE_CONT) ) != CURLE_OK) {
            std::ostringstream s;
            s << " Execute request err async retry : " << curl_easy_strerror(r) << ", " << std::string(buff_err) << std::endl;
            throw Glib::Error( Glib::Quark("CURLRequest::execute_chunk::CURL_ERROR"),  r,s.str() );
         }
    }

    const size_t r_size = std::min<size_t>(max_size, state.internal_buffer.size());
    for(size_t i =0; i < r_size; ++i){
        buffer.push_back(state.internal_buffer.front());
        state.internal_buffer.pop_front();
    }

}

void CURLRequest::finish_block(){
    if(state._pause== true){
       state._abort=true;
        curl_easy_pause(_handle, CURLPAUSE_CONT);
        state._abort=false;
        state._pause = false;
    }
    state._started = false;
}


void CURLRequest::set_requestcustom(const std::string &request_str){
    curl_easy_setopt(_handle, CURLOPT_CUSTOMREQUEST, request_str.c_str());
}


const std::vector<char> & CURLRequest::get_result(){
    return vec;
}

int CURLRequest::get_request_code(){
    long  res = -1;
    curl_easy_getinfo (_handle, CURLINFO_RESPONSE_CODE, &res);
    return (int) res;
}

void CURLRequest::disable_ssl_ca_check(){
     davix_log_warning(" disable the SSL host & peer validity .... this can lead to security issues");
     curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYPEER, 0L);
     curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYHOST, 1L);
}


void CURLRequest::get_credential(){
    Auth_code res;
    char buffer[DAVIX_BUFFER_SIZE];
    CURLcode curl_code;
    GError * tmp_err=NULL;

    davix_log_debug(" CURLRequest::get_credentialtry : to load credential or get identification");
    if(_call == NULL){
      davix_log_debug(" CURLRequest::get_credential : No identification defined, back to normal mode");
      return;
    }

    davix_log_debug(" CURLRequest::get_credential try : try to get full PEM credential ");
    res =  _call( DAVIX_FULL_PEM, buffer, _userdata , &tmp_err);
    if(  res != DAVIX_AUTH_SUCCESS && res != DAVIX_AUTH_SKIP ){
        if(!tmp_err)
            throw Glib::Error(Glib::Quark("CURLRequest::get_credential"), EINVAL, "Undefined Error while credential authentification");
        throw Glib::Error(tmp_err,false);
    }else if( res == DAVIX_AUTH_SUCCESS ){
         char * cert_path = canonicalize_file_name(buffer);
         davix_log_debug(" get a full PEM credential, try to setup libcurl : %s", cert_path);
         if (  ( curl_code = curl_easy_setopt(_handle, CURLOPT_SSLCERT, cert_path)) != CURLE_OK){
            std::ostringstream s;
            s << "Unable to use credential  "<< cert_path << ", Error : " << curl_easy_strerror(curl_code) << std::endl;
            throw Glib::Error(Glib::Quark("CURLRequest::get_credential"), EINVAL, s.str());
         }

         free(cert_path);
         davix_log_debug(" CURLRequest::get_credentialtry : credential configured with success ");
        return;
    }

    davix_log_debug(" CURLRequest::get_credentialtry : No credential defined, No auth mode");

}

void CURLRequest::clear_result(){
    vec.clear();
}

} // namespace Davix


