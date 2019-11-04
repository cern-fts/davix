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

#include <core/ContentProvider.hpp>
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
#include <utils/CompatibilityHacks.hpp>

#include "../backend/StandaloneNeonRequest.hpp"

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl

namespace Davix {

class NEONSessionWrapper {
public:
    NEONSessionWrapper(NeonRequest* r, NEONSessionFactory &factory, const Uri &uri, const RequestParams &p, DavixError **err)
        : _r(r)
    {
        _sess = factory.provideNEONSession(uri, p, err);

        if(_sess && _sess->get_ne_sess() != NULL){
            ne_hook_pre_send(_sess->get_ne_sess(), NEONSessionWrapper::runHookPreSend, (void*) this);
            ne_hook_post_headers(_sess->get_ne_sess(), NEONSessionWrapper::runHookPreReceive, (void*) this);
        }
    }

    virtual ~NEONSessionWrapper() {
        if(_sess->get_ne_sess() != NULL){
            ne_unhook_pre_send(_sess->get_ne_sess(), NEONSessionWrapper::runHookPreSend, (void*) this);
            ne_unhook_post_headers(_sess->get_ne_sess(), NEONSessionWrapper::runHookPreReceive, (void*) this);
        }
    }

    ne_session* get_ne_sess() {
        return _sess->get_ne_sess();
    }

    bool isRecycledSession() const {
        return _sess->isRecycledSession();
    }

    void do_not_reuse_this_session() {
        _sess->do_not_reuse_this_session();
    }

private:

    static void runHookPreSend(ne_request *r, void *userdata, ne_buffer *header) {
      (void) r;

      NEONSessionWrapper* wrapper = (NEONSessionWrapper*) userdata;
      BoundHooks &boundHooks = wrapper->_r->_bound_hooks;
      if(boundHooks.presendHook) {
        std::string header_line(header->data, (header->used)-1);
        boundHooks.presendHook(header_line);
      }
    }

    static void runHookPreReceive(ne_request *r, void *userdata, const ne_status *status) {
      (void) r;

      NEONSessionWrapper* wrapper = (NEONSessionWrapper*) userdata;
      BoundHooks &boundHooks = wrapper->_r->_bound_hooks;
      if(boundHooks.prereceiveHook){
        std::ostringstream header_line;
        HeaderVec headers;
        wrapper->_r->getAnswerHeaders(headers);
        header_line << "HTTP/"<< status->major_version << '.' << status->minor_version
                    << ' ' << status->code << ' ' << status->reason_phrase << '\n';

        boundHooks.prereceiveHook(header_line.str(), headers, status->code);
      }
    }

    std::unique_ptr<NEONSession> _sess;
    NeonRequest* _r;
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


NeonRequest::NeonRequest(const BoundHooks &hooks, Context& context, const Uri & uri_req) :
    BackendRequest(context, uri_req),
    _neon_sess(),
    _bound_hooks(hooks),
    _req(NULL),
    _number_try(0),
    _redirects(0),
    _total_read_size(0),
    _last_read(-1),
    req_started(false),
    req_running(false),
    _headers_configured(false),
    _accepted_202_retries(0) {
}


NeonRequest::~NeonRequest(){
    // safe destruction of the request
    resetRequest();
}

void NeonRequest::cancelSessionReuse(){
   // if session registered
   // cancel any re-use of the session
   //
   if(_neon_sess) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Connection problem: eradicate session");
       _neon_sess->do_not_reuse_this_session();
    }

    if(_neon_req) {
        _neon_req->doNotReuseSession();
    }
}



void NeonRequest::resetRequest(){
    if(req_running) {
        endRequest(NULL);
    }
    freeRequest();
}

//------------------------------------------------------------------------------
// Prepare URI & params
//------------------------------------------------------------------------------
void NeonRequest::prepareUriParams() {
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
}

//------------------------------------------------------------------------------
// Check redirect cache
//------------------------------------------------------------------------------
void NeonRequest::checkRedirectCache() {
    std::shared_ptr<Uri> redir_url;
    if(this->_params.getTransparentRedirectionSupport()) {
        redir_url = ContextExplorer::RedirectionResolverFromContext(_context).redirectionResolve(_request_type, *_current);
    }

    // performing an operation which could change the PFN? Clear all cache entries for selected URL
    if(_request_type == "DELETE" || _request_type == "MOVE") {
        ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(*_current.get());
    }

    if(redir_url.get()) {
        _current = redir_url;
    }
}

int NeonRequest::createRequest(DavixError** err){
    if(_req){
       resetRequest();
    }

    checkRedirectCache();
    prepareUriParams();

    if( instanceSession(err) < 0)
        return -1;

    _req= ne_request_create(_neon_sess->get_ne_sess(), _request_type.c_str(), _current->getPathAndQuery().c_str());
    configureRequest();

    return 0;
}

int NeonRequest::instanceSession(DavixError** err){
    DavixError * tmp_err=NULL;

    NEONSessionFactory& factory = ContextExplorer::SessionFactoryFromContext(getContext());
    _neon_sess.reset(new NEONSessionWrapper(this, factory, *_current, _params, &tmp_err));
    if(tmp_err){
        _neon_sess.reset();
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;
}

static ssize_t content_provider_callback(void* userdata, char* buffer, size_t buflen) {
    ContentProvider *provider = static_cast<ContentProvider*>(userdata);
    return provider->pullBytes(buffer, buflen);
}

void NeonRequest::configureHeaders() {
    // add custom user headers, but make sure they're only added once
    // in case of a redirect

    if(!_headers_configured) {
        std::copy(_params.getHeaders().begin(), _params.getHeaders().end(), std::back_inserter(_headers_field));
        _headers_configured = true;
    }
}

void NeonRequest::configureRequest(){

    configureHeaders();

    //--------------------------------------------------------------------------
    // Doing a PUT to Azure? Detect if this is happening, and engage
    // Azure-specific upload machinery.
    //--------------------------------------------------------------------------
    if(_content_provider && CompatibilityHacks::azureChunkedUpload(_request_type, *_current.get(), _context, _params, *_content_provider,
      &_early_termination_error)) {
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
        _neon_sess->do_not_reuse_this_session();

    if(_content_provider) {
        _content_provider->rewind();
        ne_set_request_body_provider(_req, _content_provider->getSize(),
                                     content_provider_callback, _content_provider);
    }
}

//------------------------------------------------------------------------------
// Initialize and configure _neon_req
//------------------------------------------------------------------------------
void NeonRequest::createBackendRequest() {
    configureHeaders();
    checkRedirectCache();
    prepareUriParams();

    // setup timeout
    setupDeadlineIfUnset();

    NEONSessionFactory& factory = ContextExplorer::SessionFactoryFromContext(getContext());
    _neon_req.reset(new StandaloneNeonRequest(
        factory,
        true,
        _bound_hooks,
        * (_current.get()),
        _request_type,
        _params,
        _headers_field,
        _req_flag,
        _content_provider,
        _deadline
    ));

    // configure connexion parameters for PUT request
    if( (_req_flag & RequestFlag::SupportContinue100) == true) {
        _neon_req->doNotReuseSession();
    }

}

int NeonRequest::processRedirection(int neonCode, DavixError **err){
    ne_discard_response(_req);              // Get a valid redirection, drop request content
    _last_read =0;
    ne_end_request(_req);      // submit the redirection
    if(redirectRequest(err) <0){           // accept redirection
        return -1;
    }
    return NE_RETRY;
}

int NeonRequest::startRequest(DavixError **err){
    if( createRequest(err) < 0)
            return -1;
    return negotiateRequest(err);
}

//------------------------------------------------------------------------------
// Is the given session error network-related?
//------------------------------------------------------------------------------
static bool isNetworkError(const char* sessionErr) {
    if(!sessionErr) {
        return false;
    }

    if(strstr(sessionErr, "Could not") != NULL) {
        return true;
    }

    if(strstr(sessionErr, "Connection reset by peer") != NULL) {
        return true;
    }

    if(strstr(sessionErr, "Broken pipe") != NULL) {
        return true;
    }

    return false;
}

int NeonRequest::negotiateRequest(DavixError** err){
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

            if( status == NE_ERROR && requestCleanup() && isNetworkError(ne_get_error(_neon_sess->get_ne_sess()))) {
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

                if(_content_provider && CompatibilityHacks::dynafedAssistedS3Upload(*this, *_current.get(), _context, _params, *_content_provider,
                   &_early_termination_error)) {

                    // Dynafed mechanism was engaged, end request
                    requestCleanup();
                    DavixError::clearError(err);
                    endRequest(NULL);

                    _early_termination = true;
                    if(!_early_termination_error) return 0;
                    return -1;
                }

                if(!this->_params.getTransparentRedirectionSupport()) {
                    // We don't follow redirects, give response as-is
                    end_status = NE_OK;
                    break;
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
bool NeonRequest::requestCleanup(){
    // cleanup redirection
    ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(_request_type, *_orig);

    // disable recycling
    // server supporting broken pipelining will trigger if reused
    _neon_sess->do_not_reuse_this_session();

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

//------------------------------------------------------------------------------
// We're following a redirect, store new location into the given Uri.
//------------------------------------------------------------------------------
Status NeonRequest::obtainRedirectedLocation(Uri &out) {
    if(_neon_req) {
        return _neon_req->obtainRedirectedLocation(out);
    }

    const ne_uri * new_uri = ne_redirect_location(_neon_sess->get_ne_sess());

    if(!new_uri) {
        return Status(davix_scope_http_request(), StatusCode::UriParsingError, "Impossible to get the new redirected destination");
    }

    char* dst_uri = ne_uri_unparse(new_uri);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "redirection from {} to {}", _current->getString(), dst_uri);
    out = Uri(dst_uri);
    ne_free(dst_uri);
    return Status();
}

int NeonRequest::redirectRequest(DavixError **err) {
    Uri location;
    Status st = obtainRedirectedLocation(location);

    if(!st.ok()) {
        st.toDavixError(err);
        return -1;
    }

    // setup new path & session target
    std::shared_ptr<Uri> old_uri = _current;
    _current= std::shared_ptr<Uri>(new Uri(location));
    ContextExplorer::RedirectionResolverFromContext(_context).addRedirection(_request_type, *old_uri, _current);

    // recycle old request and session
    freeRequest();
    _neon_sess.reset();

    // renew request
    req_started = false;
    // create a new couple of session + request
    if( createRequest(err) <0){
        return -1;
    }
    req_started= true;
    return 0;
}

int NeonRequest::executeRequest(DavixError** err){
    dav_ssize_t read_status=1, total_read=0;
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

   return 0;
}

int NeonRequest::beginRequest(DavixError** err){
    int ret = -1;
    _vec.clear();
    if( (ret= startRequest(err)) < 0)
        return -1;

    return ret;
}

dav_ssize_t NeonRequest::readBlock(char* buffer, dav_size_t max_size, DavixError** err){
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
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NeonRequest::readBlock read {} bytes (from buffer)", max_size);
        return max_size;
       }else{
           const dav_ssize_t n_bytes = _vec_line.size();
           std::copy(_vec_line.begin(), _vec_line.end(), buffer);
           _vec_line.clear();
           read_status = readBlock(buffer + n_bytes, max_size -n_bytes, err);
           const dav_ssize_t ret_value = (read_status >= 0)?(read_status+n_bytes):(-1);
           DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NeonRequest::readBlock read {} bytes(from partially)", ret_value);
           return ret_value;
       }
    }

    if(_neon_req) {
        Status st;
        dav_ssize_t retval = _neon_req->readBlock(buffer, max_size, st);
        if(!st.ok()) {
          st.toDavixError(err);
          return retval;
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

    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NeonRequest::readBlock read {} bytes", read_status);

    _total_read_size += read_status;
    if(_params.getTransferMonitorCb()){
        dav_ssize_t final_size = getAnswerSize();
        _params.getTransferMonitorCb()(*_current, Transfer::Read, _total_read_size, ((final_size> 0)?(final_size):(0)));
    }
    return read_status;
}

int NeonRequest::endRequest(DavixError** err){
    int status;
    (void) err;

    if(_neon_req) {
        req_started = req_running = false;
        return _neon_req->endRequest().toDavixError(err);
    }

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
int NeonRequest::getRequestCode() {
    if(_early_termination) {
        if(!_early_termination_error) return 200;
        return _early_termination_error->getStatus();
    }

    if(_neon_req) {
        return _neon_req->getStatusCode();
    }

    return ne_get_status(_req)->code;
}

bool NeonRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {
    if(_neon_req) {
        return _neon_req->getAnswerHeader(header_name, value);
    }

    if(_req){
        const char* answer_content = ne_get_response_header(_req, header_name.c_str());
        if(answer_content){
            value = answer_content;
            return true;
        }
    }
    return false;
}

size_t NeonRequest::getAnswerHeaders( HeaderVec & vec_headers) const {
    if(_neon_req) {
        return _neon_req->getAnswerHeaders(vec_headers);
    }

    if(_req){
        void * handle = NULL;
        const char* name=NULL, *value=NULL;
        while( (handle = ne_response_header_iterate(_req, handle, &name, &value)) != NULL){
            vec_headers.push_back(std::pair<std::string, std::string>(name, value));
        }
    }
    return vec_headers.size();
}

void NeonRequest::freeRequest(){
    DavixError::clearError(&_early_termination_error);
    if(_req != NULL){
        ne_request_destroy(_req);
        _req=NULL;
    }

    _neon_req.reset();
}


void NeonRequest::createError(int ne_status, DavixError **err){
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


} // namespace Davix
