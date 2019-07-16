/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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
#include <utils/davix_gcloud_utils.hpp>
#include <ne_redirect.h>
#include <ne_request.h>
#include <davix_context_internal.hpp>
#include <neon/neonsession.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/AzureIO.hpp>
#include <fileops/S3IO.hpp>
#include <core/RedirectionResolver.hpp>



namespace Davix {

class NEONSessionWrapper {
public:
    NEONSessionWrapper(NEONRequest* r, const Uri &uri, const RequestParams &p, DavixError **err)
        : _r(r)
    {
        _sess = ContextExplorer::SessionFactoryFromContext(r->getContext()).provideNEONSession(uri, p, err);

        if(_sess && _sess->get_ne_sess() != NULL){
            ne_hook_pre_send(_sess->get_ne_sess(), NEONRequest::neon_hook_pre_send, (void*)r);
            ne_hook_post_headers(_sess->get_ne_sess(), NEONRequest::neon_hook_pre_rec, (void*) r);
        }
    }

    virtual ~NEONSessionWrapper() {
        if(_sess->get_ne_sess() != NULL){
            ne_unhook_pre_send(_sess->get_ne_sess(), NEONRequest::neon_hook_pre_send, (void*)_r);
            ne_unhook_post_headers(_sess->get_ne_sess(), NEONRequest::neon_hook_pre_rec, (void*) _r);
        }
    }

    ne_session* get_ne_sess() {
        return _sess->get_ne_sess();
    }

    bool isRecycledSession() const {
        return _sess->isRecycledSession();
    }

    void disable_session_reuse() {
        _sess->disable_session_reuse();
    }

private:
    std::unique_ptr<NEONSession> _sess;
    NEONRequest* _r;
};

void configureRequestParamsProto(const Uri &uri, RequestParams &params){
    if(params.getProtocol() == RequestProtocol::Auto){
        const std::string & proto = uri.getProtocol();
        if( proto.compare(0,2,"s3") ==0){
            params.setProtocol(RequestProtocol::AwsS3);
        }else if ( proto.compare(0, 3,"dav") ==0){
            params.setProtocol(RequestProtocol::Webdav);
        }else if ( proto.compare(0, 6, "gcloud") ==0) {
            params.setProtocol(RequestProtocol::Gcloud);
        }
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
             str = std::string("(Neon): ").append(str_error);
             break;
        }
        default:
            neon_generic_error_mapper(ne_status, code, str);
    }
    DavixError::setupError(err,scope, code, str);
}



NEONRequest::NEONRequest(HttpRequest & h, Context& context, const Uri & uri_req) :
    BackendRequest(context, uri_req),
    _neon_sess(),
    _req(NULL),
    _number_try(0),
    _redirects(0),
    _total_read_size(0),
    _last_read(-1),
    _h(h),
    req_started(false),
    req_running(false),
    _accepted_202_retries(0) {
}


NEONRequest::~NEONRequest(){
    // safe destruction of the request
    resetRequest();
}

void NEONRequest::cancelSessionReuse(){
   // if session registered
   // cancel any re-use of the session
   //
   if(_neon_sess.get() != NULL){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Connection problem: eradicate session");
       _neon_sess->disable_session_reuse();
    }
}



void NEONRequest::resetRequest(){
    if(req_running) {
        endRequest(NULL);
    }
    freeRequest();
}

int NEONRequest::createRequest(DavixError** err){
    if(_req){
       resetRequest();
    }

    std::shared_ptr<Uri> redir_url;
    if(this->_params.getTransparentRedirectionSupport()) {
        redir_url = ContextExplorer::RedirectionResolverFromContext(_context).redirectionResolve(_request_type, *_current);
    }

    // performing an operation which could change the PFN? Clear all cache entries for selected URL
    if(_request_type == "DELETE" || _request_type == "MOVE") {
        ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(*_current.get());
    }

    if(redir_url.get() != NULL){
        _current.swap(redir_url);
    }

    if( instanceSession(err) < 0)
        return -1;


    // reconfigure protos
    configureRequestParamsProto(*_current, _params);

    // configure S3 params if needed
    if(_params.getProtocol() == RequestProtocol::AwsS3)
        configureS3params();

    // configure azure params if needed
    if(_params.getProtocol() == RequestProtocol::Azure)
        configureAzureParams();

    // configure gcloud params if needed
    if(_params.getProtocol() == RequestProtocol::Gcloud)
        configureGcloudParams();

    _req= ne_request_create(_neon_sess->get_ne_sess(), _request_type.c_str(), _current->getPathAndQuery().c_str());
    configureRequest();

    return 0;
}

int NEONRequest::instanceSession(DavixError** err){
    DavixError * tmp_err=NULL;
    _neon_sess.reset(new NEONSessionWrapper(this, *_current, _params, &tmp_err));
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

static dav_ssize_t iocontext_content_provider(HttpBodyProvider provider, void* udata, void* buffer, dav_size_t size) {
    return provider(udata, (char*) buffer, size);
}

void NEONRequest::configureRequest(){

    // add custom user headers, but make sure they're only added once
    // in case of a redirect
    if(!req_running) {
        std::copy(_params.getHeaders().begin(), _params.getHeaders().end(), std::back_inserter(_headers_field));
    }

    /* Doing a PUT to Azure? Horrific workaround to enforce the odd two-step
       custom Azure procedure, even when receiving a redirect. */
    if(_request_type == "PUT" &&
         _current.get()->queryParamExists("sig") &&
         _current.get()->queryParamExists("sr") &&
         _current.get()->queryParamExists("sp") && !_current.get()->fragmentParamExists("azuremechanism")) {

        IOChainContext iocontext(_context, *_current, &_params);

        using std::placeholders::_1;
        using std::placeholders::_2;

        try {
          AzureIO azureio;

          if(_fd_content > 0) {
              azureio.writeFromFd(iocontext, _fd_content, _content_len);
          }
          else if(_content_provider.callback) {
              DataProviderFun provider = std::bind(iocontext_content_provider, _content_provider.callback, _content_provider.udata, _1, _2);
              azureio.writeFromCb(iocontext, provider, _content_len);
          }
          else if(_content_ptr && _content_len > 0) {
              throw DavixException("neon request", StatusCode::InvalidArgument, "unable to use Azure PUT by providing a direct string");
          }
        }
        catch(DavixException &e) {
            e.toDavixError(&_early_termination_error);
        }

        _early_termination = true;
    }

    // setup timeout
    setupDeadlineIfUnset();

    // setup headers
    for(size_t i=0; i< _headers_field.size(); ++i){
        ne_add_request_header(_req, _headers_field[i].first.c_str(),  _headers_field[i].second.c_str());
    }
    // setup flags
    ne_set_request_flag(_req, NE_REQFLAG_EXPECT100, _params.get100ContinueSupport() &&
                        (_req_flag & RequestFlag::SupportContinue100));
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
}


int NEONRequest::processRedirection(int neonCode, DavixError **err){
    int end_status = -1;
    if (this->_params.getTransparentRedirectionSupport()) {
        if( neonCode != NE_OK
                && neonCode != NE_RETRY
                && neonCode != NE_REDIRECT){
            req_started= req_running = false;
            createError(neonCode, err);
            return -1;
        }
        ne_discard_response(_req);              // Get a valid redirection, drop request content
        _last_read =0;
        end_status = ne_end_request(_req);      // submit the redirection
        if(redirectRequest(err) <0){           // accept redirection
            return -1;
        }
        end_status = NE_RETRY;
    }
    else {
        end_status = 0;
    }
    return end_status;
}

static dav_ssize_t readFunction(int fd, void* buffer, dav_size_t size) {
    dav_ssize_t ret = ::read(fd, buffer, size);
    if(ret < 0) {
        int myerr = errno;
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CHAIN, "Error in readFunction when attempting to read from fd {}: Return code {}, errno: {}", fd, ret, myerr);
    }

    return ret;
}

int NEONRequest::startRequest(DavixError **err){
    if( createRequest(err) < 0)
            return -1;
    return negotiateRequest(err);
}

int NEONRequest::negotiateRequest(DavixError** err){
    std::string ugrs3post;
    std::string ugrpluginid;

    const uint64_t s3SizeLimit = (1024ull * 1024ull * 1024ull * 5ull);
    const int auth_retry_limit = _params.getOperationRetry();
    int code, status, end_status = NE_RETRY;
    _last_read = -1;
    _total_read_size = 0;


    DAVIX_SCOPE_TRACE(DAVIX_LOG_HTTP, negoreq);

    // check timeout
    if(checkTimeout(err) == true)
        return -1;

    if(req_started){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "Http request already started, Error");
        return -1;
    }

    req_started = req_running = true;

    while(end_status == NE_RETRY && _number_try <= auth_retry_limit) {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NEON start internal request");

        if( (status = ne_begin_request(_req)) != NE_OK && status != NE_REDIRECT){
            _last_read = -1;

            if( status == NE_ERROR
                &&  requestCleanup()
                && (
                    strstr(ne_get_error(_neon_sess->get_ne_sess()), "Could not") != NULL
                    || strstr(ne_get_error(_neon_sess->get_ne_sess()), "Connection reset by peer") != NULL
                    || strstr(ne_get_error(_neon_sess->get_ne_sess()), "Broken pipe") != NULL
                    )){
               DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_HTTP, "Connection problem, retry");
               _number_try++;
                return startRequest(err);
            }

            if( (status == NE_CONNECT || status == NE_LOOKUP) &&  requestCleanup() ){
                DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_HTTP, "Impossible to connect to {}, retry from origin {}", _current->getString(), _orig->getString());
                _number_try++;
                return startRequest(err);
            }

            req_started= req_running == false;
            createError(status, err);

            cancelSessionReuse();
            ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(_request_type, *_orig);
            return -1;
        }

        _last_read =1;
        code = getRequestCode();
        switch(code){
            case 202:
                // azure will reply with 202 when deleting files :(
                // Only activate retries on GET.
                if(_request_type != "GET") goto default_label;

                _accepted_202_retries++;
                if(_accepted_202_retries <= _params.getAcceptedRetry()) {
                  std::cerr << "DAVIX: Received 202-Accepted, sleeping for " <<
                    _params.getAcceptedRetryDelay() << " seconds before retrying. (attempt " <<
                    _accepted_202_retries << " out of " << _params.getAcceptedRetry() << ")" << std::endl;
                  sleep(_params.getAcceptedRetryDelay());
                  endRequest(NULL);
                  return startRequest(err);
                }
                goto default_label;
            case 301:
            case 302:
            case 303:
            case 307:

                if(getAnswerHeader("x-ugrs3posturl", ugrs3post) &&
                   getAnswerHeader("x-ugrpluginid", ugrpluginid) &&
                   !ugrs3post.empty() && (_content_len >= s3SizeLimit || _current->fragmentParamExists("forceMultiPart")) ) {
                    // Ugly workaround for s3 + multi-part upload + dynafed
                    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Initiating dynafed-assisted multi-part upload to S3, posturl: {}, pluginid: {}", ugrs3post, ugrpluginid);
                    IOChainContext iocontext(_context, *_current, &_params);

                    using std::placeholders::_1;
                    using std::placeholders::_2;

                    S3IO s3io;
                    DataProviderFun provider;

                    if(_fd_content > 0) {
                        provider = std::bind(readFunction, _fd_content, _1, _2);
                    }
                    else if(_content_provider.callback) {
                        provider = std::bind(iocontext_content_provider, _content_provider.callback, _content_provider.udata, _1, _2);
                    }

                    requestCleanup();
                    DavixError::clearError(err);
                    endRequest(NULL);

                    _early_termination = true;
                    s3io.performUgrS3MultiPart(iocontext, ugrs3post, ugrpluginid, provider, _content_len, &_early_termination_error);

                    if(!err) return 0;
                    return -1;
                }

                if( (end_status = processRedirection(status, err)) <0){
                   return -1;
                }

                if(_early_termination) {
                    end_status = 0;
                    break;
                }

                _number_try--;
                _redirects++;

                if(_redirects > NEON_REDIRECT_LIMIT) {
                    httpcodeToDavixError(code, davix_scope_http_request(), "Too many redirects", err);
                    return -1;
                }

                break;
            case 401: // authentification requested, do retry
            case 403:
                ne_discard_response(_req);
                end_status = ne_end_request(_req);
                _last_read = -1;

                if( end_status != NE_RETRY){
                    req_started= req_running = false;
                    clearAnswerContent();

                    _number_try++;
                    if (requestCleanup()){
                        DavixError::clearError(err);
                        endRequest(NULL);
                        return startRequest(err);
                    }

                    if(end_status == NE_OK){
                        httpcodeToDavixError(code, davix_scope_http_request(), "", err);
                        return -1;
                    }

                    createError(end_status, err);
                    return -1;
                }
                DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "(NEON) {} code -> request again ", code);
                break;
            case 501:
                 // cleanup redirection
                _number_try++;
                if( requestCleanup()){
                    // recursive call, restart request
                    endRequest(NULL);
                    return startRequest(err);
                }
                end_status = 0;
                break;
            default:
            default_label:
                if(code >= 400) {
                    httpcodeToDavixError(code, davix_scope_http_request(), "", err);
                }
                end_status = 0;
                break;

        }
        _number_try++;
    }

    if(end_status == NE_RETRY) {
        DavixError::setupError(err,davix_scope_http_request(), StatusCode::AuthenticationError,
                               "Authentication error, reached maximum number of attempts");
        return -2;
    }

    return 0;
}


// mark the session as dirty for destruction, clean any cached redirection due to error
// detect if the request comes from a cached session or have been redirected
// if it is the case, return true
//
bool NEONRequest::requestCleanup(){
    // cleanup redirection
    ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(_request_type, *_orig);

    // disable recycling
    // server supporting broken pipelining will trigger if reused
    _neon_sess->disable_session_reuse();

    // check if we had a redirection
    // if it's the case redirection can be expired, then
    if(_current != _orig || _neon_sess->isRecycledSession()){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, " ->  Error when using reycling of session/redirect : cancel and try again");
        // disable session reuse for this request, avoid picking up a session already expired
        _params.setKeepAlive(false);
        _current = _orig;
        return true;
    }
    return false;
}

int NEONRequest::redirectRequest(DavixError **err){
    std::shared_ptr<Uri> old_uri;
    const ne_uri * new_uri = ne_redirect_location(_neon_sess->get_ne_sess());
    if(!new_uri){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "Impossible to get the new redirected destination");
        return -1;
    }

    char* dst_uri = ne_uri_unparse(new_uri);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "redirection from {} to {}", _current->getString(), dst_uri);

    // setup new path & session target
    old_uri = _current;
    _current= std::shared_ptr<Uri>(new Uri(dst_uri));
    ne_free(dst_uri);
    ContextExplorer::RedirectionResolverFromContext(_context).addRedirection(_request_type, *old_uri, _current);


    // recycle old request and session
    freeRequest();
    _neon_sess.reset(NULL);

    // renew request
    req_started = false;
    // create a new couple of session + request
    if( createRequest(err) <0){
        return -1;
    }
    req_started= true;
    return 0;
}

int NEONRequest::executeRequest(DavixError** err){
    dav_ssize_t read_status=1, total_read=0;
    _internal_status = RequestStatus::kNotStarted;
    _vec.clear();

    DAVIX_SCOPE_TRACE(DAVIX_LOG_HTTP, execReq);

    if( startRequest(err) < 0){
        return -1;
    }

    if(getAnswerSize() > 0)
        _vec.reserve(std::min<size_t>(getAnswerSize(), 4194304));

    while(read_status > 0){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "NEON Read data flow");
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
        if(err && *err == NULL){
            createError(read_status, err);
        }
        return -1;
    }
    _vec.push_back('\0');

    if(_ans_size < 0){
        _ans_size = total_read;
    }

   if( endRequest(err) < 0){
       return -1;
   }

   _internal_status = RequestStatus::kCompletedOneShot;
   return 0;
}

int NEONRequest::beginRequest(DavixError** err){
    int ret = -1;
    _internal_status = RequestStatus::kNotStarted;
    _vec.clear();
    if( (ret= startRequest(err)) < 0)
        return -1;

    _internal_status = RequestStatus::kStarted;
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

    // check timeout
    if(checkTimeout(err) == true)
        return -1;

    // take from line buffer
    if(_vec_line.size() > 0){
       if( _vec_line.size() >= max_size){
        std::copy(_vec_line.begin(), _vec_line.begin() + max_size, buffer);
        _vec_line.erase(_vec_line.begin(), _vec_line.begin() + max_size);
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NEONRequest::readBlock read {} bytes (from buffer)", max_size);
        return max_size;
       }else{
           const dav_ssize_t n_bytes = _vec_line.size();
           std::copy(_vec_line.begin(), _vec_line.end(), buffer);
           _vec_line.clear();
           read_status = readBlock(buffer + n_bytes, max_size -n_bytes, err);
           const dav_ssize_t ret_value = (read_status >= 0)?(read_status+n_bytes):(-1);
           DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NEONRequest::readBlock read {} bytes(from partially)", ret_value);
           return ret_value;
       }
    }

    if(_last_read ==0){
        return 0;
    }

    _last_read= read_status= ne_read_response_block(_req, buffer, max_size );
    if(read_status <0){
       DavixError::setupError(err, davix_scope_http_request(), StatusCode::ConnectionProblem, "Invalid Read in request");
       cancelSessionReuse();
       req_running = false;
       return -1;
    }

    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NEONRequest::readBlock read {} bytes", read_status);

    _total_read_size += read_status;
    if(_params.getTransferMonitorCb()){
        dav_ssize_t final_size = getAnswerSize();
        _params.getTransferMonitorCb()(*_current, Transfer::Read, _total_read_size, ((final_size> 0)?(final_size):(0)));
    }
    return read_status;
}

int NEONRequest::endRequest(DavixError** err){
    int status;
    (void) err;

    if(_req  && req_running == true){

        if(_last_read != 0){ // if read content, discard it
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "(EndRequest)(Libneon) Operation incomplete, kill the connection");
            ne_abort_request(_req);
            cancelSessionReuse();
            _last_read = -1;

        }
        status = ne_end_request(_req);
        if(status != NE_OK && status != NE_REDIRECT) {
            DavixError* tmp_err=NULL;
            createError(status, err);
            if(tmp_err){
                DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "(EndRequest)(Libneon) Suppress broken connection {}  -> {} ", tmp_err->getStatus(), tmp_err->getErrMsg());
                cancelSessionReuse();
            }
            DavixError::clearError(&tmp_err);
        }
    }
    req_started = req_running = false;
    return 0;
}

//------------------------------------------------------------------------------
// Get response status.
//------------------------------------------------------------------------------
int NEONRequest::getRequestCode(){
    if(_early_termination) {
        if(!_early_termination_error) return 200;
        return _early_termination_error->getStatus();
    }
    return ne_get_status(_req)->code;
}

bool NEONRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {
    if(_req){
        const char* answer_content = ne_get_response_header(_req, header_name.c_str());
        if(answer_content){
            value = answer_content;
            return true;
        }
    }
    return false;
}

size_t NEONRequest::getAnswerHeaders( HeaderVec & vec_headers) const {
    if(_req){
        void * handle = NULL;
        const char* name=NULL, *value=NULL;
        while( (handle = ne_response_header_iterate(_req, handle, &name, &value)) != NULL){
            vec_headers.push_back(std::pair<std::string, std::string>(name, value));
        }
    }
    return vec_headers.size();
}

void NEONRequest::freeRequest(){
    DavixError::clearError(&_early_termination_error);
    if(_req != NULL){
        ne_request_destroy(_req);
        _req=NULL;
    }
}


void NEONRequest::createError(int ne_status, DavixError **err){
    StatusCode::Code code;
    std::string str;
    switch(ne_status){
        case NE_ERROR:
            {
             const char * neon_error = ne_get_error(_neon_sess->get_ne_sess());
             str = std::string("(Neon): ").append((neon_error)?(neon_error):"");
             if (str.find("SSL handshake failed") == std::string::npos)
                 code = StatusCode::ConnectionProblem;
             else
                 code = StatusCode::SSLError;
            }
             break;
        case NE_TIMEOUT:
            {
            // check if redirection occured, if redirection occured
            // report TimeoutRedirectionError, to allow error recovery
            if(_current != _orig){
                code = StatusCode::TimeoutRedirectionError;
                str= "Connection Timeout during redirection on ";
                str+= _current->getString();
                break;
            }
            neon_generic_error_mapper(ne_status, code, str);
            }
            break;

        default:
            neon_generic_error_mapper(ne_status, code, str);
            break;
    }
    DavixError::setupError(err, davix_scope_http_request(), code, str);
}

void NEONRequest::neon_hook_pre_send(ne_request *r, void *userdata,
                   ne_buffer *header){
    (void) r;
    NEONRequest* req = (NEONRequest*) userdata;
    RequestPreSendHook hook = req->_context.getHook<RequestPreSendHook>();
    if(hook){
        std::string header_line(header->data, (header->used)-1);
        hook(req->_h, header_line);
    }
}

void NEONRequest::neon_hook_pre_rec(ne_request *r, void *userdata,
                                    const ne_status *status){
    (void) r;
    NEONRequest* req = (NEONRequest*) userdata;
    RequestPreReceHook hook = req->_context.getHook<RequestPreReceHook>();
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
