#include "httprequest.hpp"
#include <sstream>
#include <ostream>

#include <neon/neonrequest.hpp>

namespace Davix {

HttpRequest::HttpRequest(NEONRequest* req)
{
    d_ptr = req;
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

void HttpRequest::set_parameters(const RequestParams &p){
    d_ptr->set_parameters(p);
}

void HttpRequest::add_full_request_content(const std::string &body){
    d_ptr->add_full_request_content(body);
}

int HttpRequest::getRequestCode(){
    return d_ptr->getRequestCode();
}

int HttpRequest::executeRequest(DavixError **err){
    return d_ptr->executeRequest(err);
}

int HttpRequest::execute_block(DavixError **err){
    return d_ptr->execute_block(err);
}

ssize_t HttpRequest::read_block(char *buffer, size_t max_size, DavixError **err){
    return d_ptr->read_block(buffer, max_size, err);
}

int HttpRequest::finish_block(DavixError **err){
    return d_ptr->finish_block(err);
}


void HttpRequest::clear_result(){
    d_ptr->clear_result();
}

const std::vector<char> & HttpRequest::get_result(){
    return d_ptr->get_result();
}


int httpcode_to_errno(int code)
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
            return 0;
        case 401:           /* Unauthorized */
        case 402:           /* Payment Required */
        case 407:           /* Proxy Authentication Required */
            return EPERM;
        case 301:           /* Moved Permanently */
        case 303:           /* See Other */
        case 404:           /* Not Found */
        case 410:           /* Gone */
            return ENOENT;
        case 408:           /* Request Timeout */
        case 504:           /* Gateway Timeout */
            return EAGAIN;
        case 423:           /* Locked */
        case 403:           /* Forbidden */
            return EACCES;
        case 400:           /* Bad Request */
        case 405:           /* Method Not Allowed */
        case 409:           /* Conflict */
        case 411:           /* Length Required */
        case 412:           /* Precondition Failed */
        case 414:           /* Request-URI Too Long */
        case 415:           /* Unsupported Media Type */
        case 424:           /* Failed Dependency */
        case 501:           /* Not Implemented */
            return EINVAL;
        case 413:           /* Request Entity Too Large */
        case 507:           /* Insufficient Storage */
            return ENOSPC;
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
            return EIO;
        default:
            return EIO;
    }
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
            dav_code = StatusCode::authentificationError;
            str_msg = "HTTP Authentification Error";
            break;
        case 301:           /* Moved Permanently */
        case 303:           /* See Other */
        case 404:           /* Not Found */
        case 410:           /* Gone */
            dav_code = StatusCode::fileNotFound;
            str_msg = "HTTP File not found";
            break;
        case 408:           /* Request Timeout */
        case 504:           /* Gateway Timeout */
            dav_code = StatusCode::OperationTimeout;
            str_msg = "HTTP Operation timeout";
            break;
        case 423:           /* Locked */
        case 403:           /* Forbidden */
            dav_code = StatusCode::permissionRefused;
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
            dav_code = StatusCode::ConnexionProblem;
            str_msg = "HTTP Server Error";
            break;
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
