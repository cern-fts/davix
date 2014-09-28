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

#include <davix_internal.hpp>
#include "neonrequest.hpp"

#include <utils/davix_logger_internal.hpp>
#include <libs/time_utils.h>
#include <ne_redirect.h>
#include <ne_request.h>
#include <davix_context_internal.hpp>
#include <neon/neonsession.hpp>
#include <fileops/fileutils.hpp>




namespace Davix {

class NEONSessionExtended : public NEONSession{
public:
    NEONSessionExtended(NEONRequest* r, const Uri &uri, const RequestParams &p, DavixError **err)
        : NEONSession(r->_c, uri, p, err), _r(r)
    {
        if(get_ne_sess() != NULL){
            ne_hook_pre_send(get_ne_sess(), NEONRequest::neon_hook_pre_send, (void*)r);
            ne_hook_post_headers(get_ne_sess(), NEONRequest::neon_hook_pre_rec, (void*) r);
        }
    }

    virtual ~NEONSessionExtended(){
        if(get_ne_sess() != NULL){
            ne_unhook_pre_send(get_ne_sess(), NEONRequest::neon_hook_pre_send, (void*)_r);
            ne_unhook_post_headers(get_ne_sess(), NEONRequest::neon_hook_pre_rec, (void*) _r);
        }
    }

private:
    NEONRequest* _r;
};



static void neonrequest_eradicate_session(NEONSession& sess, DavixError ** err){
    if(err && *err && (*err)->getStatus() == StatusCode::ConnectionProblem){
        DAVIX_DEBUG("Connexion problem: eradicate session.....");
        sess.disable_session_reuse();
    }
}


void neon_generic_error_mapper(int ne_status, StatusCode::Code & code, std::string & str){
    switch(ne_status){
        case NE_OK:
            code = StatusCode::OK;
            str= "Status Ok";
            break;
        case NE_LOOKUP:
             code = StatusCode::NameResolutionFailure;
             str= "Domain name resolution failed";
             break;
        case NE_AUTH:
            code = StatusCode::AuthenticationError;
            str=  "Authentification failed on server";
            break;
        case NE_PROXYAUTH:
            code = StatusCode::AuthenticationError;
            str=  "Authentification failed on proxy";
            break;
        case NE_CONNECT:
            code = StatusCode::ConnectionProblem;
            str= "Could not connect to server";
            break;
        case NE_TIMEOUT:
            code = StatusCode::ConnectionTimeout;
            str= "Connection timed out";
            break;
        case NE_FAILED:
            code = StatusCode::SessionCreationError;
            str=  "The precondition failed";
            break;
        case NE_RETRY:
            code = StatusCode::RedirectionNeeded;
            str= "Retry Request";
            break;
        default:
            code= StatusCode::UnknowError;
            str= "Unknow Error from libneon";
    }
}


// convert standard neon error to davix code
void neon_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err){
    StatusCode::Code code;
    std::string str;
    switch(ne_status){
        case NE_ERROR:
             str = std::string("Neon error: ").append(ne_get_error(sess));
             if (str.find("SSL handshake failed") == std::string::npos)
                 code = StatusCode::ConnectionProblem;
             else
                 code = StatusCode::SSLError;
             break;
        default:
            neon_generic_error_mapper(ne_status, code, str);
    }
    DavixError::setupError(err,scope, code, str);
}

// convert neon_simple_request error to davix code,
void neon_simple_req_code_to_davix_code(int ne_status, ne_session* sess, const std::string & scope, DavixError** err){
    StatusCode::Code code;
    std::string str;
    switch(ne_status){
        case NE_ERROR:{
             const char * str_error = ne_get_error(sess);
             if(strstr(str_error, "404") != NULL){
                 code = StatusCode::FileNotFound;
             }else if(strstr(str_error, "401") != NULL || strstr(str_error, "403") != NULL){
                 code = StatusCode::PermissionRefused;
             }else{
                 code = StatusCode::ConnectionProblem;
             }
             str = std::string("Neon error: ").append(str_error);
             break;
        }
        default:
            neon_generic_error_mapper(ne_status, code, str);
    }
    DavixError::setupError(err,scope, code, str);
}



NEONRequest::NEONRequest(HttpRequest & h, Context& context, const Uri & uri_req) :
    _req_flag(RequestFlag::IdempotentRequest),
    params(),
    _neon_sess(),
    _req(NULL),
    _current( new Uri(uri_req)),
    _orig(_current),
    _number_try(0),
    _last_read(-1),
    _vec(),
    _vec_line(),
    _content_ptr(),
    _content_len(0),
    _content_offset(0),
    _content_body(),
    _fd_content(-1),
    _content_provider(),
    _ans_size(-1),
    _request_type("GET"),
    _h(h),
    _c(context),
    req_started(false),
    req_running(false),
    _last_request_flag(0),
    _headers_field(){

    if(davix_get_log_level() & DAVIX_LOG_TRACE){
        ne_debug_init(stderr, NE_DBG_HTTP | NE_DBG_HTTPAUTH | NE_DBG_HTTPPLAIN | NE_DBG_HTTPBODY | NE_DBG_SSL);
    }

}




NEONRequest::~NEONRequest(){
    // safe destruction of the request
    reqReset();
}


void NEONRequest::reqReset(){
    if(req_running) {
        endRequest(NULL);
    }
    free_request();
}

int NEONRequest::create_req(DavixError** err){
    if(_req){
       reqReset();
    }

    boost::shared_ptr<Uri> redir_url = ContextExplorer::SessionFactoryFromContext(_c).redirectionResolve(_request_type, *_current);
    if(redir_url.get() != NULL){
        _current.swap(redir_url);
    }

    if( pick_sess(err) < 0)
        return -1;

    _req= ne_request_create(_neon_sess->get_ne_sess(), _request_type.c_str(), _current->getPathAndQuery().c_str());
    configure_req();

    return 0;
}

int NEONRequest::pick_sess(DavixError** err){
    DavixError * tmp_err=NULL;
    _neon_sess.reset(static_cast<NEONSession*>(new NEONSessionExtended(this, *_current, params, &tmp_err)));
    if(tmp_err){
        _neon_sess.reset(NULL);
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;
}

ssize_t NEONRequest::neon_body_content_provider(void* userdata, char* buffer, size_t buflen){
	NEONRequest* req = static_cast<NEONRequest*>(userdata);
	return (ssize_t) req->_content_provider.callback(req->_content_provider.udata, buffer, buflen);
}

void NEONRequest::configure_req(){
    // configure S3 params if needed
    if(params.getProtocol() == RequestProtocol::AwsS3)
        configureS3params();

    // setup headers
    for(size_t i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }
    // setup flags
    ne_set_request_flag(_req, NE_REQFLAG_EXPECT100, _req_flag & RequestFlag::SupportContinue100);
    ne_set_request_flag(_req, NE_REQFLAG_IDEMPOTENT, _req_flag & RequestFlag::IdempotentRequest);

    // configure connexion parameters for PUT request
    if( (_req_flag & RequestFlag::SupportContinue100) == true)
        _neon_sess->disable_session_reuse();

    if(_fd_content > 0){
        ne_set_request_body_fd(_req, _fd_content, _content_offset, _content_len);
    }else if(_content_provider.callback) {
        ne_set_request_body_provider(_req, _content_len,
                                     &neon_body_content_provider, this);
    }else if(_content_ptr && _content_len >0){
        ne_set_request_body_buffer(_req, _content_ptr, _content_len);       
    }

    const HeaderVec &  h  = params.getHeaders();
    for (HeaderVec::const_iterator i = h.begin();
         i != h.end(); i++) {
        ne_add_request_header(_req, i->first.c_str(), i->second.c_str());
      }
}


static std::string extract_bucket(const Uri & uri){
    const std::string & hostname = uri.getHost();
    std::string::const_iterator it = std::find(hostname.begin(), hostname.end(),'.');
    return std::string(hostname.begin(), it);
}

void NEONRequest::configureS3params(){
    struct tm utc_current;
    time_t t = time(NULL);
    char date[255];
    std::ostringstream ss;
    date[254]= '\0';
#ifdef HAVE_GMTIME_R	
    gmtime_r(&t, &utc_current);
#else
	struct tm* p_utc = gmtime(&t);
	memcpy(&utc_current, p_utc, sizeof(struct tm));
#endif	
    strftime(date, 254, "%a, %d %b %Y %H:%M:%S %z", &utc_current);
    // add Date header
    addHeaderField("Date", date);
    // construct Request token
    ss << _request_type << "\n"
       << "\n"          // TODO : implement Content-type and md5 parser
       << "\n"
       << date << "\n"
       << '/' << extract_bucket(*_current)  << _current->getPath();
    addHeaderField("Authorization", getAwsAuthorizationField(ss.str(), params.getAwsAutorizationKeys().first, params.getAwsAutorizationKeys().second));
}

int NEONRequest::processRedirection(int neonCode, DavixError **err){
    int end_status = -1;
    if (this->params.getTransparentRedirectionSupport()) {
        if( neonCode != NE_OK
                && neonCode != NE_RETRY
                && neonCode != NE_REDIRECT){
            req_started= req_running = false;
            neon_to_davix_code(neonCode, _neon_sess->get_ne_sess(), davix_scope_http_request(),err);
            return -1;
        }
        ne_discard_response(_req);              // Get a valid redirection, drop request content
        _last_read =0;
        end_status = ne_end_request(_req);      // submit the redirection
        if(redirect_request(err) <0){           // accept redirection
            return -1;
        }
        end_status = NE_RETRY;
    }
    else {
        end_status = 0;
    }
    return end_status;
}


int NEONRequest::startRequest(DavixError **err){        
    if( create_req(err) < 0)
            return -1;
    return negotiate_request(err);
}

int NEONRequest::negotiate_request(DavixError** err){

    const int n_limit = 10;
    int code, status, end_status = NE_RETRY;
    _last_read = -1;

    DAVIX_DEBUG(" ->   Davix negociate request ... ");
    if(req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "Http request already started, Error");
        DAVIX_DEBUG(" Davix negociate request ... <-");
        return -1;
    }

    req_started = req_running = true;

    while(end_status == NE_RETRY && _number_try < n_limit){
        DAVIX_DEBUG(" ->   NEON start internal request ... ");

        if( (status = ne_begin_request(_req)) != NE_OK && status != NE_REDIRECT){
            if( status == NE_ERROR){ // bugfix against neon keepalive problem, protection against buggy servers
                if(strstr(ne_get_error(_neon_sess->get_ne_sess()), "Could not") != NULL
                   || strstr(ne_get_error(_neon_sess->get_ne_sess()), "Connection reset by peer") != NULL){
                   DAVIX_LOG(DAVIX_LOG_VERBOSE, "Server KeepAlive problem detected, retry");
                   _number_try++;
                   redirect_cleanup();
                   return startRequest(err);
                }
            }
            if( status == NE_CONNECT && _orig != _current){
                DAVIX_LOG(DAVIX_LOG_VERBOSE, "Impossible to connect to %s, retry src", _current->getString().c_str());
                _number_try++;
                redirect_cleanup();
                return startRequest(err);
            }

            req_started= req_running == false;
            neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(),err);
            neonrequest_eradicate_session(*_neon_sess, err);
            ContextExplorer::SessionFactoryFromContext(_c).redirectionClean(_request_type, *_orig);
            DAVIX_DEBUG(" Davix negociate request ... <-");
            return -1;
        }

        _last_read =1;
        code = getRequestCode();
        switch(code){
            case 301:
            case 302:
            case 303:
            case 307:
                if( (end_status = processRedirection(status, err)) <0){
                   DAVIX_DEBUG(" Davix negociate request ... <-");
                   return -1;
                }
                break;
            case 401: // authentification requested, do retry
                ne_discard_response(_req);
                _last_read =0;
                end_status = ne_end_request(_req);

                if( end_status != NE_RETRY){
                    req_started= req_running = false;
                    clearAnswerContent();
                    if(end_status == NE_OK){
                            DavixError::setupError(err,davix_scope_http_request(),
                                                   StatusCode::AuthentificationError, "401 Unauthorized Error");
                    }else{
                        neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(),err);
                    }
                    _number_try++;
                    if (redirect_cleanup()){
                        DavixError::clearError(err);
                        endRequest(NULL);
                        return startRequest(err);
                    }

                    return -1;
                }
                DAVIX_DEBUG(" ->   NEON receive %d code, %d .... request again ... ", code, end_status);
                break;
            case 501:
            // cleanup redirection
            _number_try++;
            if( redirect_cleanup()){
                // recursive call, restart request
                endRequest(NULL);
                return startRequest(err);
            }

            return -1;

            default:
                end_status = 0;
                break;

        }
        _number_try++;
    }

    if(_number_try >= n_limit){
        DavixError::setupError(err,davix_scope_http_request(),StatusCode::AuthentificationError,
                               "Maximum number of retrial reached.");
        return -2;
    }
    DAVIX_DEBUG(" Davix negociate request ... <-");
    return 0;
}


bool NEONRequest::redirect_cleanup(){
    // cleanup redirection
    ContextExplorer::SessionFactoryFromContext(_c).redirectionClean(_request_type, *_orig);
    // disable recycling
    // server supporting broken pipelining will trigger if reused
    _neon_sess->disable_session_reuse();
    // cancel redirect, maybe outdated ? retry
    if(_current != _orig){
        DAVIX_DEBUG(" ->  Auth problem after redirect: cancel redirect and try again");
        _current = _orig;
        return true;
    }
    return false;
}

int NEONRequest::redirect_request(DavixError **err){
    boost::shared_ptr<Uri> old_uri;
    const ne_uri * new_uri = ne_redirect_location(_neon_sess->get_ne_sess());
    if(!new_uri){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "Impossible to get the new redirected destination");
        return -1;
    }

    char* dst_uri = ne_uri_unparse(new_uri);
    DAVIX_DEBUG("redirection from %s to %s", _current->getString().c_str(), dst_uri);

    // setup new path & session target
    old_uri = _current;
    _current= boost::shared_ptr<Uri>(new Uri(dst_uri));
    ne_free(dst_uri);
    ContextExplorer::SessionFactoryFromContext(_c).addRedirection(_request_type, *old_uri, _current);


    // recycle old request and session
    free_request();
    _neon_sess.reset(NULL);

    // renew request
    req_started = false;
    // create a new couple of session + request
    if( create_req(err) <0){
        return -1;
    }
    req_started= true;
    return 0;
}

int NEONRequest::executeRequest(DavixError** err){
    dav_ssize_t read_status=1, total_read=0;
    _last_request_flag =0;
    _vec.clear();

    DAVIX_DEBUG(" -> NEON start synchronous  request... ");
    if( startRequest(err) < 0){
        return -1;
    }

    if(getAnswerSize() > 0)
        _vec.reserve(std::min<size_t>(getAnswerSize(), 4194304));

    while(read_status > 0){
        DAVIX_DEBUG(" -> NEON Read data flow... ");
        size_t s = _vec.size();
        _vec.resize(s + NEON_BUFFER_SIZE);
        read_status= readBlock(&(_vec[s]), NEON_BUFFER_SIZE, err);
        if( read_status >= 0){
            if(read_status != NEON_BUFFER_SIZE){
                _vec.resize(s +  read_status);
            }
           total_read += read_status;
        }

    }

    if(read_status < 0){
        if(err && *err == NULL)
            neon_to_davix_code(read_status, _neon_sess->get_ne_sess(), davix_scope_http_request(), err);
        return -1;
    }
    _vec.push_back('\0');

    if(_ans_size < 0){
        _ans_size = total_read;
    }

   if( endRequest(err) < 0){
       return -1;
   }
    DAVIX_DEBUG(" -> End synchronous request ... ");
    _last_request_flag =1; // 1 -> syn request
    return 0;
}

int NEONRequest::beginRequest(DavixError** err){
    int ret = -1;
    _last_request_flag = 0;
    _vec.clear();
    if( (ret= startRequest(err)) < 0)
        return -1;
    _last_request_flag = 2; // 2 -> sequential req
    return ret;
}

dav_ssize_t NEONRequest::readBlock(char* buffer, dav_size_t max_size, DavixError** err){
    dav_ssize_t read_status=-1;

    if(_req == NULL){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "No request started");
        return -1;
    }

    if(max_size ==0)
        return 0;

    // take from line buffer
    if(_vec_line.size() > 0){
       if( _vec_line.size() >= max_size){
        std::copy(_vec_line.begin(), _vec_line.begin() + max_size, buffer);
        _vec_line.erase(_vec_line.begin(), _vec_line.begin() + max_size);
        DAVIX_TRACE("NEONRequest::readBlock read %ld bytes (from buffer)", max_size);
        return max_size;
       }else{
           const dav_ssize_t n_bytes = _vec_line.size();
           std::copy(_vec_line.begin(), _vec_line.end(), buffer);
           _vec_line.clear();
           read_status = readBlock(buffer + n_bytes, max_size -n_bytes, err);
           const dav_ssize_t ret_value = (read_status >= 0)?(read_status+n_bytes):(-1);
           DAVIX_TRACE("NEONRequest::readBlock read %ld bytes(from partially)", ret_value);
           return ret_value;
       }
    }

    if(_last_read ==0)
        return 0;

    _last_read= read_status= ne_read_response_block(_req, buffer, max_size );
    if(read_status <0){
       DavixError::setupError(err, davix_scope_http_request(), StatusCode::ConnectionProblem, "Invalid Read in request");
       req_running = false;
       return -1;
    }
    DAVIX_TRACE("NEONRequest::readBlock read %ld bytes", read_status);
    return read_status;
}

dav_ssize_t NEONRequest::readToFd(int fd, dav_size_t read_size, DavixError** err){
    dav_ssize_t ret=1, total=0;
    dav_size_t chunk_size = DAVIX_BLOCK_SIZE;
    read_size = (read_size==0)?(std::numeric_limits<dav_size_t>::max()):read_size;
    std::vector<char> buffer(chunk_size);

    while( (ret = readBlock(&buffer[0],
                            std::min<dav_size_t>(chunk_size,read_size),
                           err)) >0
           && ( read_size >0 )){
           if(((dav_size_t)ret) == chunk_size
                && chunk_size < DAVIX_MAX_BLOCK_SIZE){ // increase buffer size
                   chunk_size = std::min<dav_size_t>(chunk_size << 1, DAVIX_MAX_BLOCK_SIZE);
                   buffer.resize(chunk_size);
           }

           dav_ssize_t write_len = ret;
           read_size -= ret;
           total += ret;
           do{
               ret = write(fd, &buffer[0], write_len);

               if (ret == -1 && errno == EINTR) {
                   continue;
               } else if (ret < 0) {
                   DavixError::setupError(err, davix_scope_http_request(),
                                          StatusCode::SystemError, std::string("Impossible to write to fd").append(strerror(errno)));
                   return -1;
               } else {
                   write_len -= ret;
               }
           }while(write_len >0);
    }

    return (ret>= 0)?total:ret;
}


dav_ssize_t NEONRequest::readLine(char* buffer, dav_size_t max_size, DavixError** err){
    dav_ssize_t ret=-1;

    if( _vec_line.size() > 0){
       std::vector<char>::iterator it;
       it = std::find(_vec_line.begin(), _vec_line.end(), '\n');

       if( it  != _vec_line.end()){
           it ++;
           const dav_ssize_t read_size = it - _vec_line.begin();
           std::copy(_vec_line.begin(), it, buffer);
           _vec_line.erase(_vec_line.begin(), it);
           return read_size;
       }

       std::copy(_vec_line.begin(), _vec_line.end(), buffer);
       const dav_ssize_t n_bytes =  _vec_line.size();
       _vec_line.clear();
       ret = readLine(buffer + n_bytes, max_size - n_bytes, err);
       return (ret >= 0)?(ret + n_bytes):-1;
    }

    if( ( ret = readSegment(buffer, max_size, err)) >= 0){
        // search for crlf
        char* p_endline;
        p_endline = std::find(buffer, buffer+ret, '\n');
        if( p_endline < buffer+ret) p_endline++;
        _vec_line.reserve(ret - ( p_endline - buffer ));
        std::copy(p_endline, buffer + ret, std::back_inserter(_vec_line));
        *p_endline = '\0';
        return  p_endline - buffer;
    }
    return -1;
}


dav_ssize_t NEONRequest::readSegment(char* p_buff, dav_size_t size_read,  DavixError**err){
    DavixError* tmp_err=NULL;
    dav_ssize_t ret, tmp_ret;
    dav_size_t s_read= size_read;
    ret = tmp_ret = 0;
    DAVIX_TRACE("Davix::Request::readSegment: want to read %lld bytes ", size_read);

    do{
        tmp_ret= readBlock(p_buff, s_read, &tmp_err);
        if(tmp_ret > 0){ // tmp_ret bytes readed
            ret += tmp_ret;
        }
        if(ret > 0 && ret < (dav_ssize_t) size_read){
            p_buff+= tmp_ret;
            s_read -= tmp_ret;
        }
    }while( tmp_ret > 0
            &&  ret < (dav_ssize_t) size_read);

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return ret;
}

int NEONRequest::endRequest(DavixError** err){
    int status;
    (void) err;

    if(_req  && req_running == true){
        req_running = false;
        if(_last_read > 0){ // if read content, discard it
            DAVIX_TRACE("Discard remaining content....");
            ne_discard_response(_req);
            _last_read =0;
        }
        if( (status = ne_end_request(_req)) != NE_OK){
            DavixError* tmp_err=NULL;
            if(_neon_sess.get() != NULL)
                neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(), &tmp_err);
            if(tmp_err)
                DAVIX_DEBUG("NEONRequest::endRequest -> error %d Error closing request -> %s ", tmp_err->getStatus(), tmp_err->getErrMsg().c_str());
            DavixError::clearError(&tmp_err);
        }
    }
    req_started = req_running = false;
    return 0;
}

void NEONRequest::clearAnswerContent(){
    _vec.clear();
}



int NEONRequest::getRequestCode(){
    return ne_get_status(_req)->code;
}

const char* NEONRequest::getAnswerContent(){
    if(_last_request_flag == 1)
        return (const char*) &(_vec.at(0));
    return NULL;
}

dav_ssize_t NEONRequest::getAnswerSizeFromHeaders() const{
    std::string str_file_size;
    long size=-1;
    if( getAnswerHeader(ans_header_content_length, str_file_size)){
        size =  strtol(str_file_size.c_str(), NULL, 10);
    }
    if( size == -1 || size ==  LONG_MAX){
       DAVIX_TRACE("Bad server answer: %s Invalid, impossible to determine answer size", ans_header_content_length.c_str());
       size = -1;
    }
    return static_cast<dav_ssize_t>(size);
}

dav_ssize_t NEONRequest::getAnswerSize() const{
    if(_ans_size < 0)
        _ans_size = getAnswerSizeFromHeaders();
    return _ans_size;
}

bool NEONRequest::getAnswerHeader(const std::string &header_name, std::string &value) const{
    if(_req){
        const char* answer_content = ne_get_response_header(_req, header_name.c_str());
        if(answer_content){
            value = answer_content;
            return true;
        }
    }
    return false;
}


size_t NEONRequest::getAnswerHeaders( HeaderVec & vec_headers) const{
    if(_req){
        void * handle = NULL;
        const char* name=NULL, *value=NULL;
        while( (handle = ne_response_header_iterate(_req, handle, &name, &value)) != NULL){
            vec_headers.push_back(std::pair<std::string, std::string>(name, value));
        }
    }
    return vec_headers.size();
}




void NEONRequest::setRequestBody(const std::string & body){
    DAVIX_DEBUG("NEONRequest : add request content of size %s ", body.c_str());
    _content_body = std::string(body);
    _content_ptr = (char*) _content_body.c_str();
    _content_len = strlen(_content_ptr);
    _fd_content = -1;
}

void NEONRequest::setRequestBody(const void *buffer, dav_size_t len){
    _content_ptr = (char*) buffer;
    _content_len = len;
    _fd_content = -1;
}


void NEONRequest::setRequestBody(int fd, dav_off_t offset, dav_size_t len){
    _fd_content = fd;
    _content_ptr = NULL;
    _content_len = len;
    _content_offset = offset;
}

void NEONRequest::setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata){
    _content_len      = len;
    _content_provider.callback = provider;
    _content_provider.udata    = udata;
}


void NEONRequest::free_request(){
    if(_req != NULL){
        ne_request_destroy(_req);
        _req=NULL;
    }
}

void NEONRequest::neon_hook_pre_send(ne_request *r, void *userdata,
                   ne_buffer *header){
    (void) r;
    NEONRequest* req = (NEONRequest*) userdata;
    RequestPreSendHook hook = req->_c.getHook<RequestPreSendHook>();
    if(hook){
        std::string header_line(header->data, (header->used)-1);
        hook(req->_h, header_line);
    }
}

void NEONRequest::neon_hook_pre_rec(ne_request *r, void *userdata,
                                    const ne_status *status){
    (void) r;
    NEONRequest* req = (NEONRequest*) userdata;
    RequestPreReceHook hook = req->_c.getHook<RequestPreReceHook>();
    if(hook){
        std::ostringstream header_line;
        HeaderVec headers;
        req->getAnswerHeaders(headers);
        header_line << "HTTP/"<< status->major_version << '.' << status->minor_version
                    << ' ' << status->code << ' ' << status->reason_phrase << '\n';

        hook(req->_h, header_line.str(), headers, status->code);
    }
}


} // namespace Davix
