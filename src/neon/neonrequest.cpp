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
#include <backend/SessionFactory.hpp>
#include <curl/StandaloneCurlRequest.hpp>

#include "../backend/StandaloneNeonRequest.hpp"

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl

namespace Davix {

void configureRequestParamsProto(const Uri &uri, RequestParams &params){
    if(params.getProtocol() == RequestProtocol::Auto){
        const std::string & proto = uri.getProtocol();
        if( proto.compare(0,2,"s3") ==0){
            params.setProtocol(RequestProtocol::AwsS3);
        }else if ( proto.compare(0, 3,"dav") ==0){
            params.setProtocol(RequestProtocol::Webdav);
        }else if ( proto.compare(0, 6, "gcloud") ==0) {
            params.setProtocol(RequestProtocol::Gcloud);
        }else if (proto.compare(0, 3, "cs3")==0){
            params.setProtocol(RequestProtocol::CS3);
        }
    }
}

void neon_generic_error_mapper(int ne_status, StatusCode::Code & code, std::string & str, const std::string &wwwAuth){
    switch(ne_status){
        case NE_OK:
            code = StatusCode::OK;
            str = "Status Ok";
            break;
        case NE_LOOKUP:
            code = StatusCode::NameResolutionFailure;
            str = "Domain name resolution failed";
            break;
        case NE_AUTH:
            code = StatusCode::AuthenticationError;
            str = "Authentication failed on server";
            break;
        case NE_PROXYAUTH:
            code = StatusCode::AuthenticationError;
            str = "Authentication failed on proxy";
            break;
        case NE_CONNECT:
            code = StatusCode::ConnectionProblem;
            str = "Could not connect to server";
            break;
        case NE_TIMEOUT:
            code = StatusCode::ConnectionTimeout;
            str = "Connection timed out";
            break;
        case NE_FAILED:
            code = StatusCode::SessionCreationError;
            str = "The precondition failed";
            break;
        case NE_RETRY:
            code = StatusCode::RedirectionNeeded;
            str = "Retry Request";
            break;
        default:
            code = StatusCode::UnknownError;
            str = "Unknown Error from libneon";
    }

    if(!wwwAuth.empty()) {
        str += "(WWW-Authenticate: ";
        str += wwwAuth;
        str += ")";
    }
}


NeonRequest::NeonRequest(const BoundHooks &hooks, Context& context, const Uri & uri_req) :
    BackendRequest(context, uri_req),
    _bound_hooks(hooks),
    _number_try(0),
    _redirects(0),
    _total_read_size(0),
    _headers_configured(false),
    _accepted_202_retries(0) {
}


NeonRequest::~NeonRequest(){
    // safe destruction of the request
    freeRequest();
}

void NeonRequest::cancelSessionReuse(){
   // if session registered
   // cancel any re-use of the session
   //
    if(_standalone_req) {
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Connection problem: eradicate session");
        _standalone_req->doNotReuseSession();
    }
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

    // configure swift params if needed
    if(_params.getProtocol() == RequestProtocol::Swift)
        configureSwiftParams();
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

void NeonRequest::configureHeaders() {
    // add custom user headers, but make sure they're only added once
    // in case of a redirect

    if(!_headers_configured) {
        std::copy(_params.getHeaders().begin(), _params.getHeaders().end(), std::back_inserter(_headers_field));
        _headers_configured = true;
    }
}

//------------------------------------------------------------------------------
// Should we use libcurl?
//------------------------------------------------------------------------------
static bool useLibcurl() {
    const char *opt = getenv("DAVIX_USE_LIBCURL");
    if(opt) {
        if(opt[0] == '1' || opt[0] == 'Y' || opt[0] == 'y') {
            return true;
        }

        if(opt[0] == '0' || opt[0] == 'N' || opt[0] == 'n') {
            return false;
        }
    }

#ifdef LIBCURL_BACKEND_BY_DEFAULT
        return true;
#else
        return false;
#endif
}

//------------------------------------------------------------------------------
// Initialize standalone request
//------------------------------------------------------------------------------
void NeonRequest::initStandaloneRequest() {
    if(useLibcurl()) {
        CurlSessionFactory& factory = ContextExplorer::SessionFactoryFromContext(getContext()).getCurl();
        _standalone_req.reset(new StandaloneCurlRequest(
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
    }
    else {
        NEONSessionFactory& factory = ContextExplorer::SessionFactoryFromContext(getContext()).getNeon();
        _standalone_req.reset(new StandaloneNeonRequest(
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
    }
}

//------------------------------------------------------------------------------
// Initialize and configure _standalone_req
//------------------------------------------------------------------------------
void NeonRequest::createBackendRequest() {
    configureHeaders();
    checkRedirectCache();
    prepareUriParams();

    //--------------------------------------------------------------------------
    // Doing a PUT to Azure? Detect if this is happening, and engage
    // Azure-specific upload machinery.
    //--------------------------------------------------------------------------
    if(_content_provider && CompatibilityHacks::azureChunkedUpload(_request_type, *_current.get(), _context, _params, *_content_provider,
      &_early_termination_error)) {
        _early_termination = true;
        return;
    }

    // setup timeout
    setupDeadlineIfUnset();

    // setup standalone req object
    initStandaloneRequest();

    // configure connexion parameters for PUT request
    if( (_req_flag & RequestFlag::SupportContinue100) == true) {
        _standalone_req->doNotReuseSession();
    }
}

int NeonRequest::processRedirection(DavixError **err){
    if(redirectRequest(err) <0){           // accept redirection
        return -1;
    }
    return NE_RETRY;
}

int NeonRequest::startRequest(DavixError **err){
    createBackendRequest();
    return negotiateRequest(err);
}

int NeonRequest::negotiateRequest(DavixError** err){
    const int auth_retry_limit = _params.getOperationRetry();
    int code, end_status = NE_RETRY;
    _total_read_size = 0;


    DAVIX_SCOPE_TRACE(DAVIX_LOG_HTTP, negoreq);

    // check timeout
    if(checkTimeout(err) == true)
        return -1;

    while(end_status == NE_RETRY && _number_try <= auth_retry_limit) {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "NEON start internal request");

        Status st = _standalone_req->startRequest();

        if(!st.ok()) {
            _number_try++;
            if(_number_try <= auth_retry_limit) {
                DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_HTTP, "Connection problem, retry");
                requestCleanup();
                return startRequest(err);
            }

            st.toDavixError(err);

            cancelSessionReuse();
            ContextExplorer::RedirectionResolverFromContext(_context).redirectionClean(_request_type, *_orig);
            return -1;
        }

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
                    // We don't follow redirects, give response as-is.
                    return NE_OK;
                }

                _redirects++;
                if(_redirects > NEON_REDIRECT_LIMIT) {
                    httpcodeToDavixError(code, davix_scope_http_request(), "Too many redirects", err);
                    return -1;
                }

                if( (end_status = processRedirection(err)) <0){
                   return -1;
                }

                if(_early_termination) {
                    end_status = 0;
                    break;
                }

                _number_try--;


                break;
            case 401: // authentication requested, do retry
            case 403:
                clearAnswerContent();

                _number_try++;
                if (_number_try <= auth_retry_limit && requestCleanup()){
                    DavixError::clearError(err);
                    endRequest(NULL);
                    return startRequest(err);
                }

                httpcodeToDavixError(code, davix_scope_http_request(), "", err);
                return -1;
            default:
            default_label:
                if(code >= 400) {
                    httpcodeToDavixError(code, davix_scope_http_request(), "", err);
                }
                return 0;

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
    if(_standalone_req) {
        _standalone_req->doNotReuseSession();
    }

    // check if we had a redirection
    // if it's the case redirection can be expired, then
    if(_current != _orig || _standalone_req->isRecycledSession()){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, " ->  Error when using reycling of session/redirect : cancel and try again");
        // disable session reuse for this request, avoid picking up a session already expired
        _params.setKeepAlive(false);
        _current = _orig;
        return true;
    }
    return false;
}

int NeonRequest::redirectRequest(DavixError **err) {
    Uri location;
    Status st = _standalone_req->obtainRedirectedLocation(location);

    if(!st.ok()) {
        st.toDavixError(err);
        return -1;
    }

    // Fill in host details in case location is relative [DMC-1209]
    if (location.getProtocol().empty() && location.getHost().empty()) {
      location = Uri::fromRelativePath(*(_current.get()), location.getString());
    }

    // setup new path & session target
    std::shared_ptr<Uri> old_uri = _current;
    _current= std::shared_ptr<Uri>(new Uri(location));
    ContextExplorer::RedirectionResolverFromContext(_context).addRedirection(_request_type, *old_uri, _current);

    // recycle old request and session
    freeRequest();

    // create a new couple of session + request
    createBackendRequest();
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
    if(_standalone_req) {
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::AlreadyRunning, "Http request already started, Error");
        return -1;
    }

    int ret = -1;
    _vec.clear();
    if( (ret= startRequest(err)) < 0)
        return -1;

    return ret;
}

dav_ssize_t NeonRequest::readBlock(char* buffer, dav_size_t max_size, DavixError** err){
    dav_ssize_t read_status=-1;

    if(!_standalone_req) {
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

    if(_standalone_req) {
        Status st;
        dav_ssize_t retval = _standalone_req->readBlock(buffer, max_size, st);
        if(!st.ok()) {
          st.toDavixError(err);
        }
        return retval;
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
    if(!_standalone_req) {
      DavixError::setupError(err, davix_scope_http_request(), StatusCode::InvalidArgument, "Request not started");
      return -1;
    }

    Status st = _standalone_req->endRequest();

    if(!st.ok()) {
        st.toDavixError(err);
    }

    return st.okAsInt();
}

//------------------------------------------------------------------------------
// Get response status.
//------------------------------------------------------------------------------
int NeonRequest::getRequestCode() {
    if(_early_termination) {
        if(!_early_termination_error) return 200;
        return _early_termination_error->getStatus();
    }

    if(_standalone_req) {
        return _standalone_req->getStatusCode();
    }

    return 0;
}

bool NeonRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {
    if(_standalone_req) {
        return _standalone_req->getAnswerHeader(header_name, value);
    }

    return false;
}

size_t NeonRequest::getAnswerHeaders( HeaderVec & vec_headers) const {
    if(_standalone_req) {
        return _standalone_req->getAnswerHeaders(vec_headers);
    }

    return vec_headers.size();
}

void NeonRequest::freeRequest(){
    DavixError::clearError(&_early_termination_error);
    _standalone_req.reset();
}

void NeonRequest::createError(int ne_status, DavixError **err){
    StatusCode::Code code;
    std::string str;
    std::string wwwAuth;
    _standalone_req->getAnswerHeader("WWW-Authenticate", wwwAuth);

    switch(ne_status){
        case NE_ERROR:
            {
             str = std::string("(Neon): ").append(_standalone_req->getSessionError());
             if (str.find("SSL handshake failed") == std::string::npos)
                 code = StatusCode::ConnectionProblem;
             else
                 code = StatusCode::SSLError;
            }
             break;
        case NE_TIMEOUT:
            {
            // check if redirection occurred, if redirection occurred
            // report TimeoutRedirectionError, to allow error recovery
            if(_current != _orig){
                code = StatusCode::TimeoutRedirectionError;
                str = "Connection Timeout during redirection on ";
                str+= _current->getString();
                break;
            }
            neon_generic_error_mapper(ne_status, code, str, wwwAuth);
            }
            break;

        default:
            neon_generic_error_mapper(ne_status, code, str, wwwAuth);
            break;
    }
    DavixError::setupError(err, davix_scope_http_request(), code, str);
}


} // namespace Davix
