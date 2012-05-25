#ifndef DAVIX_CURLREQUEST_H
#define DAVIX_CURLREQUEST_H

#include <vector>
#include <curl/curl.h>

#include <global_def.h>
#include <httprequest.h>


namespace Davix {

class CURLRequest : public Davix::HttpRequest
{
public:
    CURLRequest(CURL* handle, RequestType typ, const std::string & url);
    CURLRequest(CURL* handle, RequestType typ, const std::string & url,
                davix_auth_callback call, void* userdata);
    virtual ~CURLRequest();

    virtual int execute_sync();  // throw(Glib::Error)

    virtual int execute_chunk(off_t max_size);  // throw(Glib::Error)
    /**
      CURL implementation of \ref Request::get_result
     */
    virtual const std::vector<char> & get_result();


    /**
      CURL implementation of \ref HTTPRequest::add_header_field
     */
    virtual void add_header_field(const std::string & field, const std::string & value);

    /**
            CURL implementation of \ref HTTPRequest::set_requestcustom
    */
    void set_requestcustom(const std::string & request_str);

    /**
        CURL implementation of Request::get_request_code
     */
    virtual int get_request_code() ;

    /**
      implementation of HttpRequest::disable_ssl_ca_check
    */
    virtual void disable_ssl_ca_check();

    virtual void get_credential();

protected:
    CURL* _handle;
    RequestType _typ;
    std::string _url;
    char * buff_err;
    std::vector<char> vec;
    struct curl_slist * headers_fields;
    davix_auth_callback _call;
    void * _userdata;

    static size_t write_data_curl( char *ptr, size_t size, size_t nmemb, void *userdata);

    void _init(CURL* handle, RequestType typ, const std::string & url);
};

} // namespace Davix

#endif // DAVIX_CURLREQUEST_H
