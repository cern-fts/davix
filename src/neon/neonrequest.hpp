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


class NEONRequest : public HttpRequest
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
    virtual void addHeaderField(const std::string & field, const std::string & value);
    /**
     * set the request command to execute ( GET, POST, PUT, PROPFIND )
     */
    virtual void setRequestMethod(const std::string & request_str);

    /**
      Execute the given request and return result to the buffer result
      @return 0 on success
     */
    virtual int execute_sync(DavixError** err) ;

    virtual void add_full_request_content(const std::string & body);

    virtual int execute_block(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
    */
    virtual ssize_t read_block(char* buffer, size_t max_size,DavixError** err);
    /**
      finish an already started request
     */
    virtual int finish_block(DavixError** err);
    /**
      get a reference to the current result for synchronous full request
     */
    virtual const std::vector<char> & get_result();
    /**
      clear the current result
    */
    virtual void clear_result();



    /**
      @brief current HTTP answer code
     */
    virtual int getRequestCode();

    /**
      reimplement authentification
    */
    virtual int try_set_pkcs12_cert(const char * filename_pkcs12, const char* passwd, DavixError** err);
    virtual int try_set_login_passwd(const char *login, const char *passwd, DavixError** err);

protected:
    ne_session *    _sess;
    ne_request * _req;
    std::string  _path;
    std::vector<char> _vec;
    std::string _content_body;
    std::string _request_type;
    NEONSessionFactory* _f;
    bool req_started, req_running;
    DavixError* last_error;

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


    /**
      main libneon callback for clicert
     */
    static void provide_clicert_fn(void *userdata, ne_session *sess,
                              const ne_ssl_dname *const *dnames,
                              int dncount);
    static int provide_login_passwd_fn(void *userdata, const char *realm, int attempt,
                                char *username, char *password);

    friend int davix_set_login_passwd_auth(davix_auth_t token, const char* login, const char* passwd, GError** err);

};

/**
  translate a neon function status to a string and a errno code
 */
std::string  translate_neon_status(int ne_status, ne_session* sess, int* errno_code);

void neon_to_davix_code(int ne_status,ne_session* sess, const std::string & scope, DavixError** err);


} // namespace Davix

#endif // DAVIX_NEONREQUEST_H
