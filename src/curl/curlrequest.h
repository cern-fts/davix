#ifndef DAVIX_CURLREQUEST_H
#define DAVIX_CURLREQUEST_H

#include <vector>
#include <curl/curl.h>

#include <containers/vector_chunk.h>
#include <global_def.h>
#include <httprequest.h>


namespace Davix {



struct CURLTransferState{
    CURLTransferState(){
        clear();
    }
    void clear(){
        _pause = false;
        _started= false;
        _abort = false;
        internal_buffer.clear();
    }

    size_t _max_size_chunk;
    bool _pause;
    bool _abort;
    bool _started;
    std::deque<char> internal_buffer;
};

class CURLRequest : public Davix::HttpRequest
{
public:
    CURLRequest(CURL* handle, RequestType typ, const std::string & url);
    CURLRequest(CURL* handle, RequestType typ, const std::string & url,
                davix_auth_callback call, void* userdata);
    virtual ~CURLRequest();

    virtual int execute_sync();  // throw(Glib::Error)

    /**
      Launch request in block mode
    */
    virtual void execute_block(size_t buff_size = BUFFER_SIZE);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
      @throw Glib::Error
    */
    virtual void read_block(std::vector<char> & buffer, size_t max_size);
    /**
      finish an already started request
     */
    virtual void finish_block();

    /**
      CURL implementation of \ref Request::get_result
     */
    virtual const std::vector<char> & get_result();

    /**
      implementation of \ref Request::get_result();
      */
    virtual void clear_result();


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
    std::vector<char> vec; // results
    struct curl_slist * headers_fields; // headers fields to configure for curl
    davix_auth_callback _call; // auth callback
    void * _userdata; // internal user data for auth callback

    CURLTransferState state; // internal state for async transfer

    static size_t write_data_curl( char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t write_data_curl_chunk_new(char *ptr, size_t size, size_t nmemb, void *userdata);

    void _init(CURL* handle, RequestType typ, const std::string & url);
};

} // namespace Davix

#endif // DAVIX_CURLREQUEST_H
