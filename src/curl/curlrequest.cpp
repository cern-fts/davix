#include "curlrequest.h"
#include <string>
#include <cstring>
#include <errno.h>
#include <glibmm/error.h>
#include <glibmm/quark.h>

namespace Davix {


CURLRequest::CURLRequest(CURL* handle, RequestType typ, const std::string & url) : _url(url), _typ(typ), _handle(handle)
{
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str()); // setup url to contact
    buff_err = new char[CURL_ERROR_SIZE+1];
    memset(buff_err, '\0',CURL_ERROR_SIZE+1 );
    curl_easy_setopt(handle,CURLOPT_ERRORBUFFER, buff_err); //enable verbose error string
    curl_easy_setopt(handle,CURLOPT_FOLLOWLOCATION,TRUE); // eable redirection support
    headers_fields = NULL; // reset the overload for the header fields
}

CURLRequest::~CURLRequest(){
    curl_easy_cleanup(_handle);
    curl_slist_free_all(headers_fields);
    delete[] buff_err;
}

size_t CURLRequest::write_data_curl(char *ptr, size_t size, size_t nmemb, void *userdata){
    const size_t n_bytes = size * nmemb;
    std::vector<char>* vec = (std::vector<char>*) userdata;


    for(size_t i = 0; i < n_bytes; ++i){
        vec->push_back(ptr[i]);
    }
    return n_bytes;
}

void CURLRequest::add_header_field(const std::string &field, const std::string &value){
    std::string field_line(field);
    field_line.append(": ");
    field_line.append(value);
    headers_fields = curl_slist_append(headers_fields, field_line.c_str());
}


void CURLRequest::set_requestcustom(const std::string &request_str){
    curl_easy_setopt(_handle,CURLOPT_CUSTOMREQUEST,request_str.c_str());
}

int CURLRequest::execute_sync(){
     curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, &write_data_curl);
     curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &vec);
     curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, headers_fields);

     CURLcode r;
     if(( r = curl_easy_perform(_handle) ) != CURLE_OK){
         throw Glib::Error( Glib::Quark("CURLRequest::execute_sync::CURL_ERROR"),  r,
                         std::string(curl_easy_strerror(r)) + std::string(", ")+ std::string(buff_err));
     }
     return 0;
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
     davix_log_warning(" disable the SSL host & peer validity .... can lead to security issue");
     curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYPEER, 0L);
     curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYHOST, 1L);
}




} // namespace Davix


