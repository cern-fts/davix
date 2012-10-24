#ifndef DAVIX_NEONREQUEST_H
#define DAVIX_NEONREQUEST_H

#include <vector>
#include <utility>
#include <queue>
#include <neon/ne_request.h>
#include <neon/ne_auth.h>
#include <neon/neonsessionfactory.hpp>

#include <global_def.hpp>
#include <httprequest.hpp>

namespace Davix {

#define NEON_BUFFER_SIZE 65000

class NEONSessionFactory;


class NEONRequest
{
public:
    NEONRequest(NEONSessionFactory* f, ne_session * sess,  const std::string & path);
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

    void set_parameters(const RequestParams &p ){
        params = p;
    }


    //  Execute the given request and return result to the buffer result
    //  @return 0 on success
    int execute_sync(DavixError** err) ;

    void add_full_request_content(const std::string & body);

    int execute_block(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
    */
    ssize_t read_block(char* buffer, size_t max_size,DavixError** err);
    /**
      finish an already started request
     */
    int finish_block(DavixError** err);
    /**
      get a reference to the current result for synchronous full request
     */
    const std::vector<char> & get_result();
    /**
      clear the current result
    */
    void clear_result();



    /**
      @brief current HTTP answer code
     */
    int getRequestCode();


    // auth method support
    int do_pkcs12_cert_authentification(const char * filename_pkcs12, const char* passwd, DavixError** err);
    int do_login_passwd_authentification(const char *login, const char *passwd, DavixError** err);

protected:
    // request parameters
    RequestParams params;
    // neon internal field
    ne_session *    _sess;
    ne_request * _req;
    std::string  _path;
    std::vector<char> _vec;
    std::string _content_body;
    std::string _request_type;
    NEONSessionFactory* _f;
    bool req_started, req_running;
    DavixError* auth_last_error;

    std::vector< std::pair<std::string, std::string > > _headers_field;

    /** temporary field used for login/password authentification
     */
    std::string _passwd, _login;

    void configure_sess();
    void configure_req();

    int create_req(DavixError** err);

    int negotiate_request(DavixError** err);

    void free_request();
    /**
      internal, try to authentification with pkcs12 credential
    */
    int try_pkcs12_authentification(ne_session *sess, const ne_ssl_dname *const *dnames, DavixError** err);


private:
    static void provide_clicert_fn(void *userdata, ne_session *sess,
                              const ne_ssl_dname *const *dnames,
                              int dncount);
    static int provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password);

    friend int davix_auth_set_login_passwd(davix_auth_t token, const char* login, const char* passwd, GError** err);

};


//
// translate a neon error code to a davix one
//
void neon_to_davix_code(int ne_status,ne_session* sess, const std::string & scope, DavixError** err);


} // namespace Davix

#endif // DAVIX_NEONREQUEST_H
