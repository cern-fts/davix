/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef DAVIX_NEONREQUEST_H
#define DAVIX_NEONREQUEST_H

#include <vector>
#include <utility>
#include <memory>
#include <string>

#include <ne_request.h>
#include <ne_auth.h>
#include <neon/neonsessionfactory.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
 
#include <request/httprequest.hpp>
#include <neon/neonsession.hpp>

namespace Davix {

#define NEON_BUFFER_SIZE        65000


class NEONSessionFactory;
class NEONSession;
class HttpRequest;
class NEONSessionExtended;

struct ContentProviderContext {
    ContentProviderContext(): callback(NULL), udata(NULL) {}
    HttpBodyProvider callback;
    void *udata;
};


class NEONRequest : protected NonCopyable
{
public:
    int _req_flag;
public:
    NEONRequest(HttpRequest &h, Context& f, const Uri & uri_req);
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

    void setRequestBody(const std::string & body);

    void setRequestBody(const void * buffer, dav_size_t len);

    void setRequestBody(int fd, dav_off_t offset, dav_size_t len);

    void setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata);

    int beginRequest(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
    */
    dav_ssize_t readBlock(char* buffer, dav_size_t max_size,DavixError** err);

    dav_ssize_t readSegment(char* buffer, dav_size_t size_read,  DavixError**err);

    dav_ssize_t readLine(char* buffer, dav_size_t max_size, DavixError** err);

    dav_ssize_t readToFd(int fd, dav_size_t read_size, DavixError** err);

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
    dav_ssize_t getAnswerSize() const;

    /**
      clear the current result
    */
    void clearAnswerContent();

    int getRequestCode();

    bool getAnswerHeader(const std::string &header_name, std::string &value) const;

    size_t getAnswerHeaders( std::vector<std::pair<std::string, std::string > > & vec_headers) const;



    // auth method support
    int do_pkcs12_cert_authentification(const char * filename_pkcs12, const char* passwd, DavixError** err);
    int do_login_passwd_authentification(const char *login, const char *passwd, DavixError** err);

private:

    // request parameters
    RequestParams params;
    // neon internal field
    boost::scoped_ptr<NEONSession> _neon_sess;
    // request options flag


    ne_request * _req;
    boost::shared_ptr<Uri>  _current, _orig;
    // read info
    dav_ssize_t _last_read;

    std::vector<char> _vec;
    std::vector<char> _vec_line;

    // request content;
    char* _content_ptr;
    dav_size_t _content_len;
    dav_off_t _content_offset;
    std::string _content_body;
    int _fd_content;
    ContentProviderContext _content_provider;

    // answer length
    mutable dav_ssize_t _ans_size;
    // Request string
    std::string _request_type;
    HttpRequest & _h;
    Context& _c;
    bool req_started, req_running;
    int _last_request_flag;

    std::vector< std::pair<std::string, std::string > > _headers_field;

    ////////////////////////////////////////////
    // Private Members
    dav_ssize_t getAnswerSizeFromHeaders() const;

    int startRequest(DavixError** err);

    int processRedirection(int neonCode, DavixError** err); // analyze and process redirection if needed

    void reqReset();

    int pick_sess(DavixError** err);

    void configure_req();

    void configureS3params();

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

    NEONRequest(const NEONRequest & req);
    NEONRequest & operator=(const NEONRequest & req);

	static ssize_t neon_body_content_provider(void* userdata, char* buffer, size_t buflen);

    static void neon_hook_pre_send(ne_request *req, void *userdata,
                                           ne_buffer *header);

    static void neon_hook_pre_rec(ne_request *req, void *userdata,
                                        const ne_status *status);

    friend class HttpRequest;
    friend class NEONSessionExtended;
};


//
// translate a neon error code to a davix one
//
void neon_to_davix_code(int ne_status,ne_session* sess, const std::string & scope, DavixError** err);


void neon_simple_req_code_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err);


} // namespace Davix

#endif // DAVIX_NEONREQUEST_H
