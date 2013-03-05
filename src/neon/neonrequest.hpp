#ifndef DAVIX_NEONREQUEST_H
#define DAVIX_NEONREQUEST_H

#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <queue>
#include <ne_request.h>
#include <ne_auth.h>
#include <neon/neonsessionfactory.hpp>

 
#include <httprequest.hpp>
#include <neon/neonsession.hpp>

namespace Davix {

#define NEON_BUFFER_SIZE 65000

class NEONSessionFactory;
class NEONSession;


struct ContentProviderContext {
    ContentProviderContext(): callback(NULL), udata(NULL) {}
    HttpBodyProvider callback;
    void *udata;
};


class NEONRequest
{
public:
    NEONRequest(NEONSessionFactory& f, const Uri & uri_req);
    virtual ~NEONRequest();

    /**
      add a personalized header to the header request
      replace an existing one if already exist
      remove one if empty
      @param field : add a header field to the current http request or replace a existing one
      @param value : value of the field to set

    */
    void addHeaderField(const std::string & field, const std::string & value){
        _headers_field.push_back(std::pair<std::string, std::string> (field, value));
    }

    /**
     * set the request command to execute ( GET, POST, PUT, PROPFIND )
     */
    void setRequestMethod(const std::string & request_str){
        _request_type = request_str;
    }

    void setParameters(const RequestParams &p ){
        params = p;
    }


    //  Execute the given request and return result to the buffer result
    //  @return 0 on success
    int executeRequest(DavixError** err) ;

    void setRequestBodyString(const std::string & body);

    void setRequestBodyBuffer(const void * buffer, size_t len);

    void setRequestBodyFileDescriptor(int fd, off_t offset, size_t len);

    void setRequestBodyCallback(HttpBodyProvider provider, size_t len, void* udata);

    int beginRequest(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
    */
    ssize_t readBlock(char* buffer, size_t max_size,DavixError** err);


    ssize_t readLine(char* buffer, size_t max_size, DavixError** err);

    /**
      finish an already started request
     */
    int endRequest(DavixError** err);
    /**
      get a reference to the current result for synchronous full request
     */
    const char* getAnswerContent();
    /**
     * get content length
     **/
    ssize_t getAnswerSize() const;

    /**
      clear the current result
    */
    void clearAnswerContent();



    //
    int getRequestCode();

    bool getAnswerHeader(const std::string &header_name, std::string &value) const;


    // auth method support
    int do_pkcs12_cert_authentification(const char * filename_pkcs12, const char* passwd, DavixError** err);
    int do_login_passwd_authentification(const char *login, const char *passwd, DavixError** err);

protected:
    // request parameters
    RequestParams params;
    // neon internal field
    std::auto_ptr<NEONSession> _neon_sess;

    ne_request * _req;
    Uri  _current, _orig;
    std::vector<char> _vec;

    // request content;
    char* _content_ptr;
    size_t _content_len;
    off_t _content_offset;
    std::string _content_body;
    int _fd_content;
    ContentProviderContext _content_provider;


    std::string _request_type;
    NEONSessionFactory& _f;
    bool req_started, req_running;
    int _last_request_flag;

    std::vector< std::pair<std::string, std::string > > _headers_field;

    int pick_sess(DavixError** err);
    void configure_req();

    // create initial neon request object
    int create_req(DavixError** err);

    // negociate the request : authentification, redirection, name resolution
    int negotiate_request(DavixError** err);

    // redirection logic
    int redirect_request(DavixError** err);

    void free_request();
    /**
      internal, try to authentification with pkcs12 credential
    */
    int try_pkcs12_authentification(ne_session *sess, const ne_ssl_dname *const *dnames, DavixError** err);


private:

    NEONRequest(const NEONRequest & req);
    NEONRequest & operator=(const NEONRequest & req);


};


//
// translate a neon error code to a davix one
//
void neon_to_davix_code(int ne_status,ne_session* sess, const std::string & scope, DavixError** err);


void neon_simple_req_code_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err);


} // namespace Davix

#endif // DAVIX_NEONREQUEST_H
