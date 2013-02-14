#include "neonrequest.hpp"

#include <cstring>
#include <logger/davix_logger_internal.h>
#include <libs/time_utils.h>
#include <ne_redirect.h>
#include <ne_request.h>
#include <neon/neonsession.hpp>

namespace Davix {



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
            code = StatusCode::AuthentificationError;
            str=  "Authentification failed on server";
            break;
        case NE_PROXYAUTH:
            code = StatusCode::AuthentificationError;
            str=  "Authentification failed on proxy";
        case NE_CONNECT:
            code = StatusCode::ConnectionProblem;
            str= "Could not connect to server";
            break;
        case NE_TIMEOUT:
            code = StatusCode::ConnectionTimeout;
            str= "Connection timed out";
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
             str = std::string("Neon error : ").append(ne_get_error(sess));
             code = StatusCode::ConnectionProblem;
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


NEONRequest::NEONRequest(NEONSessionFactory& f, const Uri & uri_req) :
    params(),
    _neon_sess(NULL),
    _req(NULL),
    _current(uri_req),
    _orig(uri_req),
    _vec(),
    _content_ptr(),
    _content_len(0),
    _content_offset(0),
    _content_body(),
    _fd_content(-1),
    _content_provider(),
    _request_type("GET"),
    _f(f),
    req_started(false),
    req_running(false),
    _headers_field(){
}




NEONRequest::~NEONRequest(){
    // safe destruction of the request
    if(req_running) endRequest(NULL);
    free_request();

}

int NEONRequest::create_req(DavixError** err){
    if(_req != NULL || req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "Http request already started, Error");
        return -1;
    }

    if( pick_sess(err) < 0)
        return -1;

    _req= ne_request_create(_neon_sess->get_ne_sess(), _request_type.c_str(), _current.getPathAndQuery().c_str());
    configure_req();

    return 0;
}

int NEONRequest::pick_sess(DavixError** err){
    DavixError * tmp_err=NULL;
    _neon_sess = std::auto_ptr<NEONSession>(new NEONSession(_f, _current, params, &tmp_err) );
    if(tmp_err){
        _neon_sess.reset(NULL);
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;
}

void NEONRequest::configure_req(){
    for(size_t i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }


    if(_fd_content > 0){
        ne_set_request_body_fd(_req, _fd_content, _content_offset, _content_len);
    }else if(_content_provider.callback) {
        ne_set_request_body_provider(_req, _content_len,
                                     _content_provider.callback, _content_provider.udata);
    }else if(_content_ptr && _content_len >0){
        ne_set_request_body_buffer(_req, _content_ptr, _content_len);       
    }

   // ne_set_request_flag(_req, NE_REQFLAG_EXPECT100, 1);
}

int NEONRequest::negotiate_request(DavixError** err){

    const int n_limit = 10;
    int code, status, end_status = NE_RETRY;
    int n =0;

    DAVIX_DEBUG(" ->   Davix negociate request ... ");
    if(req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "Http request already started, Error");
        DAVIX_DEBUG(" Davix negociate request ... <-");
        return -1;
    }

    req_started = req_running= true;

    while(end_status == NE_RETRY && n < n_limit){
        DAVIX_DEBUG(" ->   NEON start internal request ... ");

        if( (status = ne_begin_request(_req)) != NE_OK && status != NE_REDIRECT){
            if( status == NE_ERROR && strstr(ne_get_error(_neon_sess->get_ne_sess()), "Could not") != NULL ){ // bugfix against neon keepalive problem
               DAVIX_DEBUG("Connexion close, retry...");
               n++;
               continue;
            }

            req_started= req_running == false;
            neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(),err);
            DAVIX_DEBUG(" Davix negociate request ... <-");
            return -1;
        }

        code = getRequestCode();
        switch(code){
            case 301:
            case 302:
            case 307:
                if (this->params.getTransparentRedirectionSupport()) {
                    if( end_status != NE_OK
                            && end_status != NE_RETRY
                            && end_status != NE_REDIRECT){
                        req_started= req_running = false;
                        neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(),err);
                        DAVIX_DEBUG(" Davix negociate request ... <-");
                        return -1;
                    }
                    ne_discard_response(_req);              // Get a valid redirection, drop request content
                    end_status = ne_end_request(_req);      // submit the redirection
                    if(redirect_request(err) <0){           // accept redirection
                        DAVIX_DEBUG(" Davix negociate request ... <-");
                        return -1;
                    }
                    end_status = NE_RETRY;
                }
                else {
                    end_status = 0;
                }
                break;
            case 401: // authentification requested, do retry
                ne_discard_response(_req);
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
                    return -1;
                }
                DAVIX_DEBUG(" ->   NEON receive %d code, %d .... request again ... ", code, end_status);
                break;
            default:
                end_status = 0;
                break;

        }
        n++;
    }

    if(n >= n_limit){
        DavixError::setupError(err,davix_scope_http_request(),StatusCode::AuthentificationError,
                               "Maximum number of retrial reached.");
        return -2;
    }
    DAVIX_DEBUG(" Davix negociate request ... <-");
    return 0;
}

int NEONRequest::redirect_request(DavixError **err){
    const ne_uri * new_uri = ne_redirect_location(_neon_sess->get_ne_sess());
    if(!new_uri){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "Impossible to get the new redirected destination");
        return -1;
    }

    char* dst_uri = ne_uri_unparse(new_uri);
    DAVIX_DEBUG("redirection from %s://%s/%s to %s", ne_get_scheme(_neon_sess->get_ne_sess()),
                      ne_get_server_hostport(_neon_sess->get_ne_sess()), _current.getPathAndQuery().c_str(), dst_uri);

    // setup new path & session target
    _current= Uri(dst_uri);
    ne_free(dst_uri);

    // recycle old request and session
    _neon_sess.reset(NULL);
    free_request();

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
    ssize_t read_status=1;
    DavixError* tmp_err=NULL;

    if( create_req(&tmp_err) < 0){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    DAVIX_DEBUG(" -> NEON start synchronous  request... ");
    if( negotiate_request(&tmp_err) < 0){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

    while(read_status > 0){
        DAVIX_DEBUG(" -> NEON Read data flow... ");
        size_t s = _vec.size();
        _vec.resize(s + NEON_BUFFER_SIZE);
        read_status= readBlock(&(_vec[s]), NEON_BUFFER_SIZE, &tmp_err);
        if( read_status >= 0 && read_status != NEON_BUFFER_SIZE){
           _vec.resize(s +  read_status);
        }

    }

    // push a last NULL char for safety
    _vec.push_back('\0');

    if(read_status < 0){
        if(!tmp_err)
            neon_to_davix_code(read_status, _neon_sess->get_ne_sess(), davix_scope_http_request(), &tmp_err);
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

   if( endRequest(&tmp_err) < 0){
       DavixError::propagateError(err, tmp_err);
       return -1;
   }

    DAVIX_DEBUG(" -> End synchronous request ... ");
    return 0;
}

int NEONRequest::beginRequest(DavixError** err){
    DavixError* tmp_err=NULL;
    int ret = -1;
    ret= create_req(&tmp_err);


    if( ret >=0){
       ret= negotiate_request(&tmp_err);
    }

    if(ret <0)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

ssize_t NEONRequest::readBlock(char* buffer, size_t max_size, DavixError** err){
    ssize_t read_status=-1;

    if(_req == NULL){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "No request started");
        return -1;
    }
    read_status= ne_read_response_block(_req, buffer, max_size );
    if(read_status <0){
       DavixError::setupError(err, davix_scope_http_request(), StatusCode::ConnectionProblem, "Invalid Read in request");
       req_running = false;
       return -1;
    }
    return read_status;
}

int NEONRequest::endRequest(DavixError** err){
    int status;

    if(_req  && req_running == true){
        req_running = false;
        if( (status = ne_end_request(_req)) != NE_OK){
            DavixError* tmp_err=NULL;
            if(_neon_sess.get() != NULL)
                neon_to_davix_code(status, _neon_sess->get_ne_sess(), davix_scope_http_request(), &tmp_err);
            if(tmp_err)
                DAVIX_DEBUG("NEONRequest::endRequest -> error %d Error closing request -> %s ", tmp_err->getStatus(), tmp_err->getErrMsg().c_str());
            DavixError::clearError(&tmp_err);
        }
    }
    return 0;
}

void NEONRequest::clearAnswerContent(){
    _vec.clear();
}



int NEONRequest::getRequestCode(){
    return ne_get_status(_req)->code;
}

const char* NEONRequest::getAnswerContent(){
    return (const char*) &(_vec.at(0));
}

size_t NEONRequest::getAnswerSize() const{
    return _vec.size()-1;
}

bool NEONRequest::getAnswerHeader(const std::string &header_name, std::string &value){
    const char* answer_content = ne_get_response_header(_req, header_name.c_str());
    if(answer_content){
        value = answer_content;
        return true;
    }
    return false;
}




void NEONRequest::setRequestBodyString(const std::string & body){
    DAVIX_DEBUG("NEONRequest : add request content of size %s ", body.c_str());
    _content_body = std::string(body);
    _content_ptr = (char*) _content_body.c_str();
    _content_len = strlen(_content_ptr);
    _fd_content = -1;
}

void NEONRequest::setRequestBodyBuffer(const void *buffer, size_t len){
    _content_ptr = (char*) buffer;
    _content_len = len;
    _fd_content = -1;
}


void NEONRequest::setRequestBodyFileDescriptor(int fd, off_t offset, size_t len){
    _fd_content = fd;
    _content_ptr = NULL;
    _content_len = len;
    _content_offset = offset;
}

void NEONRequest::setRequestBodyCallback(HttpBodyProvider provider, size_t len, void* udata){
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

} // namespace Davix
