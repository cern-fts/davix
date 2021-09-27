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

#include <neon/neonrequest.hpp>
#include <davix_context_internal.hpp>
#include <request/httprequest.hpp>

namespace Davix {

//------------------------------------------------------------------------------
// TODO: Explain why the distinction between
// NEONRequest / NeonRequest / BackendRequest is necessary (ABI compatibility)
//------------------------------------------------------------------------------
class NEONRequest {
public:
  NEONRequest(BackendRequest *req) : _req(req) {}

  virtual ~NEONRequest() {
    delete _req;
  }

  BackendRequest* get() {
    return _req;
  }

private:
  BackendRequest* _req;
};

typedef NEONRequest WrappedBackendRequest;

template <typename...> struct WhichType;


static WrappedBackendRequest* createBackendRequest(HttpRequest* req, Context & context, const Uri & uri) {
    BoundHooks boundHooks;

    RequestPreSendHook presendHook = context.getHook<RequestPreSendHook>();
    RequestPreReceHook prereceiveHook = context.getHook<RequestPreReceHook>();

    using std::placeholders::_1;
    using std::placeholders::_2 ;
    using std::placeholders::_3;

    if(presendHook) {
        boundHooks.presendHook = std::bind(presendHook, std::ref(*req), _1);
    }

    if(prereceiveHook) {
        boundHooks.prereceiveHook = std::bind(prereceiveHook, std::ref(*req), _1, _2, _3);
    }

    return new WrappedBackendRequest(new NeonRequest(boundHooks, context, uri));
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

HttpRequest::HttpRequest(WrappedBackendRequest* req) {
  std::cerr << "Usage of HttpRequest::HttpRequest(NEONRequest* req) is deprecated!" << std::endl;
}

HttpRequest::HttpRequest(Context & context, const Uri & uri, DavixError** err) :
    d_ptr(createBackendRequest(this, context, uri)) {

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Create HttpRequest for {}", uri.getString());
    if(uri.getStatus() != StatusCode::OK){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, fmt::format(" {} is not a valid HTTP or Webdav URL", uri));
    }
}

HttpRequest::HttpRequest(Context & context, const std::string & url, DavixError** err) {
    Uri uri(url);
    d_ptr = createBackendRequest(this, context, uri);
    if(uri.getStatus() != StatusCode::OK){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, fmt::format(" {} is not a valid HTTP or Webdav URL", uri));
    }
}

HttpRequest::~HttpRequest()
{
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Destroy HttpRequest");
    delete d_ptr;
}

void HttpRequest::setRequestMethod(const std::string &request_str){
    d_ptr->get()->setRequestMethod(request_str);
}


void HttpRequest::addHeaderField(const std::string &field, const std::string &value){
    d_ptr->get()->addHeaderField(field, value);
}

void HttpRequest::setParameters(const RequestParams &p){
    d_ptr->get()->setParameters(p);
}

void HttpRequest::setRequestBody(const std::string &body){
    d_ptr->get()->setRequestBody(body);
}

void HttpRequest::setRequestBody(const void *buffer, dav_size_t len_buff){
    d_ptr->get()->setRequestBody(buffer, len_buff);
}

void HttpRequest::setRequestBody(int fd, dav_off_t offset, dav_size_t len){
    d_ptr->get()->setRequestBody(fd, offset, len);
}

void HttpRequest::setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata){
    d_ptr->get()->setRequestBody(provider, len, udata);
}

void HttpRequest::setRequestBody(ContentProvider &provider) {
    d_ptr->get()->setRequestBody(provider);
}

int HttpRequest::getRequestCode(){
    return d_ptr->get()->getRequestCode();
}

int HttpRequest::executeRequest(DavixError **err){
    TRY_DAVIX{
        runPreRunHook();
        return d_ptr->get()->executeRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}

int HttpRequest::beginRequest(DavixError **err){
    TRY_DAVIX{
        // triggers Hooks
        runPreRunHook();
        return d_ptr->get()->beginRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}

dav_ssize_t HttpRequest::readBlock(char *buffer, dav_size_t max_size, DavixError **err){
    TRY_DAVIX{
        return d_ptr->get()->readBlock(buffer, max_size, err);
    }CATCH_DAVIX(err)
    return -1;
}

dav_ssize_t HttpRequest::readBlock(std::vector<char> & buffer, dav_size_t max_size, DavixError **err){
    dav_ssize_t ret=-1, v_size = (dav_ssize_t) buffer.size();
    TRY_DAVIX{

        buffer.resize(v_size + max_size);
        ret = readBlock(((char*) &buffer[0])+ v_size, max_size, err);
        buffer.resize(v_size + ( (ret > 0)?ret:0));
    }CATCH_DAVIX(err)
    return ret;
}

dav_ssize_t HttpRequest::readSegment(char* buffer, dav_size_t max_size, DavixError** err){
    return d_ptr->get()->readSegment(buffer, max_size, false, err);
}

dav_ssize_t HttpRequest::readLine(char *buffer, dav_size_t max_size, DavixError **err){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Davix::Request::readLine want to read a line of max {} chars", max_size);
    dav_ssize_t ret= -1;
    TRY_DAVIX{
        if( (ret =  d_ptr->get()->readLine(buffer, max_size, err)) >= 0){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Davix::Request::readLine got {} chars", ret);
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Davix::Request::readLine content\n[[{}]]\n", std::string(buffer, ret));
        }
    }CATCH_DAVIX(err)
    return ret;
}

void HttpRequest::discardBody(DavixError** err){
    char buffer[1024];
    dav_ssize_t read;
    TRY_DAVIX{
        do {
            read = d_ptr->get()->readSegment(buffer, sizeof(buffer), false, err);
        } while (read > 0 && *err == NULL);
    }CATCH_DAVIX(err)
}

dav_ssize_t HttpRequest::readToFd(int fd, DavixError** err){
    TRY_DAVIX{
        return d_ptr->get()->readToFd(fd, 0, err);
    }CATCH_DAVIX(err)
    return -1;
}


dav_ssize_t HttpRequest::readToFd(int fd, dav_size_t read_size, DavixError** err){
    TRY_DAVIX{
        return d_ptr->get()->readToFd(fd, read_size, err);
    }CATCH_DAVIX(err)
    return -1;
}

int HttpRequest::endRequest(DavixError **err){
    TRY_DAVIX{
        return d_ptr->get()->endRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}


void HttpRequest::clearAnswerContent(){
    d_ptr->get()->clearAnswerContent();
}

bool HttpRequest::getAnswerHeader(const std::string &header_name, std::string &value) const{
    return d_ptr->get()->getAnswerHeader(header_name, value);
}

size_t HttpRequest::getAnswerHeaders( HeaderVec & vec_headers) const{
    return d_ptr->get()->getAnswerHeaders(vec_headers);
}

const char* HttpRequest::getAnswerContent(){
    return d_ptr->get()->getAnswerContent();
}

std::vector<char> & HttpRequest::getAnswerContentVec(){
    return d_ptr->get()->getAnswerContentVec();
}


/// get content length
dav_ssize_t HttpRequest::getAnswerSize() const{
    return d_ptr->get()->getAnswerSize();
}

time_t HttpRequest::getLastModified() const{
    return d_ptr->get()->getLastModified();
}


HttpCacheToken* HttpRequest::extractCacheToken()const{
    return NULL;
}

void HttpRequest::useCacheToken(const HttpCacheToken *token){
    (void) token;
}

/// set a HttpRequest flag
void HttpRequest::setFlag(const RequestFlag::RequestFlag flag, bool value){
    d_ptr->get()->setFlag(flag, value);
}

/// get a HttpRequest flag value
bool HttpRequest::getFlag(const RequestFlag::RequestFlag flag){
    return d_ptr->get()->getFlag(flag);
}

void HttpRequest::runPreRunHook(){
    // triggers Hooks
    RequestPreRunHook hook = d_ptr->get()->getContext().getHook<RequestPreRunHook>();

    if(hook){
        hook(d_ptr->get()->getParameters(), *this, *(d_ptr->get()->getOriginalUri()));
    }
}


///////////////////////////////////////////////////
///// Simplified request GET
////////////////////////////////////////////////////

GetRequest::GetRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{

}


///////////////////////////////////////////////////
///// Simplified request PUT
////////////////////////////////////////////////////

PutRequest::PutRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{
    setRequestMethod("PUT");
    setFlag(RequestFlag::SupportContinue100, true);
    setFlag(RequestFlag::IdempotentRequest, false);
}

///////////////////////////////////////////////////
///// Simplified request POST
////////////////////////////////////////////////////

PostRequest::PostRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{
    setRequestMethod("POST");
    setFlag(RequestFlag::IdempotentRequest, false);
}

///////////////////////////////////////////////////
///// Simplified request HEAD
////////////////////////////////////////////////////

HeadRequest::HeadRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{
    setRequestMethod("HEAD");
}


///////////////////////////////////////////////////
///// Simplified request DELETE
////////////////////////////////////////////////////

DeleteRequest::DeleteRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{
    setFlag(RequestFlag::IdempotentRequest, false);
    setRequestMethod("DELETE");
}

///////////////////////////////////////////////////
///// Simplified request PROPFIND
////////////////////////////////////////////////////

PropfindRequest::PropfindRequest(Context & context, const Uri & uri, DavixError** err) :
    HttpRequest(context, uri, err)
{
    setRequestMethod("PROPFIND");
}


void httpcodeToDavixError(int code, const std::string &scope, const std::string & end_message, StatusCode::Code & dav_code, std::string & err_msg){
    std::string str_msg = "Status Ok";
    switch (code) {
        case 200:           /* OK */
        case 201:           /* Created */
        case 202:           /* Accepted */
        case 203:           /* Non-Authoritative Information */
        case 204:           /* No Content */
        case 206:           /* partial content*/
        case 205:           /* Reset Content */
        case 207:           /* Multi-Status */
        case 304:           /* Not Modified */
            dav_code = StatusCode::OK;
            str_msg = "Success";
            break;
        case 401:           /* Unauthorized */
        case 402:           /* Payment Required */
        case 407:           /* Proxy Authentication Required */
            dav_code = StatusCode::AuthenticationError;
            str_msg = "Authentication Error";
            break;
        case 303:           /* See Other */
        case 404:           /* Not Found */
        case 410:           /* Gone */
            dav_code = StatusCode::FileNotFound;
            str_msg = "File not found";
            break;
        case 408:           /* Request Timeout */
        case 504:           /* Gateway Timeout */
            dav_code = StatusCode::OperationTimeout;
            str_msg = "Operation timeout";
            break;
        case 409:
            if (scope == davix_scope_mkdir_str()){
                dav_code = StatusCode::FileNotFound;
                str_msg = "Conflict, File not Found";
            } else{
                dav_code = StatusCode::FileExist;
                str_msg = "Conflict, File Exist";
            }
            break;
        case 423:           /* Locked */
        case 403:           /* Forbidden */
            dav_code = StatusCode::PermissionRefused;
            str_msg = "Permission refused";
            break;
        break;
        case 405:           /* Method Not Allowed */
            if (scope == davix_scope_mkdir_str()){
                dav_code = StatusCode::FileExist;
                str_msg = "Method Not Allowed, File Exist";
            } else{
                dav_code = StatusCode::PermissionRefused; // Technically is a EPERM
                str_msg = "Method Not Allowed, Permission refused";
            }
            break;
        case 400:           /* Bad Request */
        case 411:           /* Length Required */
        case 412:           /* Precondition Failed */
        case 414:           /* Request-URI Too Long */
        case 415:           /* Unsupported Media Type */
        case 424:           /* Failed Dependency */
        case 501:           /* Not Implemented */
        case 413:           /* Request Entity Too Large */
            dav_code = StatusCode::ConnectionProblem;
            str_msg = "Server Error";
            break;
        case 507:           /* Insufficient Storage */
            dav_code = StatusCode::InsufficientStorage;
            str_msg = "Insufficient Storage";
            break;
        case 301:           /* Moved Permanently */
        case 300:           /* Multiple Choices */
        case 302:           /* Found */
            dav_code = StatusCode::RedirectionNeeded;
            str_msg = "Redirection requested, transparent redirection disabled";
            break;
        case 305:           /* Use Proxy */
        case 306:           /* (Unused) */
        case 307:           /* Temporary Redirect */
        case 406:           /* Not Acceptable */
        case 416:           /* Requested Range Not Satisfiable */
        case 417:           /* Expectation Failed */
        case 422:           /* Unprocessable Entity */
        case 500:           /* Internal Server Error */
        case 502:           /* Bad Gateway */
        case 503:           /* Service Unavailable */
        case 505:           /* HTTP Version Not Supported */
        default:
            dav_code = StatusCode::UnknownError;
            std::ostringstream ss;
            ss << "Unexpected server error: " << code;
            str_msg = ss.str().c_str();
            break;
    }

    err_msg.assign(fmt::format("HTTP {} : {} {}", code, str_msg, end_message));
}

void httpcodeToDavixError(int code, const std::string & scope, const std::string & end_message, DavixError** err){
    StatusCode::Code davix_code;
    std::string err_msg;
    httpcodeToDavixError(code, scope, end_message, davix_code, err_msg);
    DavixError::setupError(err, scope, davix_code, err_msg);
}


void httpcodeToDavixException(int code, const std::string & scope, const std::string & end_message){
    StatusCode::Code davix_code;
    std::string err_msg;
    httpcodeToDavixError(code, scope, end_message, davix_code, err_msg);
    throw DavixException(scope, davix_code, err_msg);
}

bool httpcodeIsValid(int code)
{
    switch (code) {
        case 200:           /* OK */
        case 201:           /* Created */
        case 202:           /* Accepted */
        case 203:           /* Non-Authoritative Information */
        case 204:           /* No Content */
        case 206:           /* partial content */
        case 205:           /* Reset Content */
        case 207:           /* Multi-Status */
        case 304:           /* Not Modified */
            return true;
        default:
            return false;
    }
}


} // namespace Davix
