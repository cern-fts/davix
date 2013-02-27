#include <config.h>
#include <sstream>
#include <ostream>

#include <neon/neonrequest.hpp>
#include <davix_context_internal.hpp>
#include "httprequest.hpp"

namespace Davix {

HttpRequest::HttpRequest(NEONRequest* req) : d_ptr(req)
{
}

HttpRequest::HttpRequest(Context & context, const Uri & uri, DavixError** err) :
    d_ptr(new NEONRequest(ContextExplorer::SessionFactoryFromContext(context),uri)){

    if(uri.getStatus() != StatusCode::OK){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "impossible to parse " + uri.getString() + " ,not a valid HTTP or Webdav URL");
    }
}

HttpRequest::HttpRequest(Context & context, const std::string & url, DavixError** err) :
    d_ptr(NULL){
    Uri uri(url);
    d_ptr= new NEONRequest(ContextExplorer::SessionFactoryFromContext(context), uri);
    if(uri.getStatus() != StatusCode::OK){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, "impossible to parse " + uri.getString() + " ,not a valid HTTP or Webdav URL");
    }
}

HttpRequest::~HttpRequest()
{
    delete d_ptr;
}




void HttpRequest::setRequestMethod(const std::string &request_str){
    d_ptr->setRequestMethod(request_str);
}


void HttpRequest::addHeaderField(const std::string &field, const std::string &value){
    d_ptr->addHeaderField(field, value);
}

void HttpRequest::setParameters(const RequestParams &p){
    d_ptr->setParameters(p);
}

void HttpRequest::setRequestBodyString(const std::string &body){
    d_ptr->setRequestBodyString(body);
}

void HttpRequest::setRequestBodyBuffer(const void *buffer, size_t len_buff){
    d_ptr->setRequestBodyBuffer(buffer, len_buff);
}

void HttpRequest::setRequestBodyFileDescriptor(int fd, off_t offset, size_t len){
    d_ptr->setRequestBodyFileDescriptor(fd, offset, len);
}

void HttpRequest::setRequestBodyCallback(HttpBodyProvider provider, size_t len, void* udata){
    d_ptr->setRequestBodyCallback(provider, len, udata);
}

int HttpRequest::getRequestCode(){
    return d_ptr->getRequestCode();
}

int HttpRequest::executeRequest(DavixError **err){
    return d_ptr->executeRequest(err);
}

int HttpRequest::beginRequest(DavixError **err){
    return d_ptr->beginRequest(err);
}

ssize_t HttpRequest::readBlock(char *buffer, size_t max_size, DavixError **err){
    return d_ptr->readBlock(buffer, max_size, err);
}

ssize_t HttpRequest::readLine(char *buffer, size_t max_size, DavixError **err){
    return d_ptr->readLine(buffer, max_size, err);
}

int HttpRequest::endRequest(DavixError **err){
    return d_ptr->endRequest(err);
}


void HttpRequest::clearAnswerContent(){
    d_ptr->clearAnswerContent();
}

bool HttpRequest::getAnswerHeader(const std::string &header_name, std::string &value) const{
    return d_ptr->getAnswerHeader(header_name, value);
}

const char* HttpRequest::getAnswerContent(){
    return d_ptr->getAnswerContent();
}

/// get content length
size_t HttpRequest::getAnswerSize() const{
    return d_ptr->getAnswerSize();
}


void httpcodeToDavixCode(int code, const std::string & scope, const std::string & end_message, DavixError** err){
    StatusCode::Code dav_code;
    std::string str_msg;
    switch (code) {
        case 200:           /* OK */
        case 201:           /* Created */
        case 202:           /* Accepted */
        case 203:           /* Non-Authoritative Information */
        case 204:           /* No Content */
        case 205:           /* Reset Content */
        case 207:           /* Multi-Status */
        case 304:           /* Not Modified */
            dav_code = StatusCode::OK;
            str_msg ="Status Ok";
            break;
        case 401:           /* Unauthorized */
        case 402:           /* Payment Required */
        case 407:           /* Proxy Authentication Required */
            dav_code = StatusCode::AuthentificationError;
            str_msg = "HTTP Authentification Error";
            break;
        case 303:           /* See Other */
        case 404:           /* Not Found */
        case 410:           /* Gone */
            dav_code = StatusCode::FileNotFound;
            str_msg = "HTTP File not found";
            break;
        case 408:           /* Request Timeout */
        case 504:           /* Gateway Timeout */
            dav_code = StatusCode::OperationTimeout;
            str_msg = "HTTP Operation timeout";
            break;
        case 423:           /* Locked */
        case 403:           /* Forbidden */
            dav_code = StatusCode::PermissionRefused;
            str_msg = "HTTP Permission refused";
            break;
        break;
        case 400:           /* Bad Request */
        case 405:           /* Method Not Allowed */
        case 409:           /* Conflict */
        case 411:           /* Length Required */
        case 412:           /* Precondition Failed */
        case 414:           /* Request-URI Too Long */
        case 415:           /* Unsupported Media Type */
        case 424:           /* Failed Dependency */
        case 501:           /* Not Implemented */
        case 413:           /* Request Entity Too Large */
        case 507:           /* Insufficient Storage */
            dav_code = StatusCode::ConnectionProblem;
            str_msg = "HTTP Server Error";
            break;
        case 301:           /* Moved Permanently */
        case 206:           /* Partial Content */
        case 300:           /* Multiple Choices */
        case 302:           /* Found */
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
            dav_code = StatusCode::UnknowError;
            str_msg = "HTTP unexcepted Server Error";
            break;
    }

    std::ostringstream ss;
    ss << "HTTP Error "<< code << " " << str_msg << std::endl;
    DavixError::setupError(err, scope, dav_code, ss.str());
}

bool httpcodeIsValid(int code)
{
    switch (code) {
        case 200:           /* OK */
        case 201:           /* Created */
        case 202:           /* Accepted */
        case 203:           /* Non-Authoritative Information */
        case 204:           /* No Content */
        case 205:           /* Reset Content */
        case 207:           /* Multi-Status */
        case 304:           /* Not Modified */
            return true;
        default:
            return false;
    }
}


} // namespace Davix
