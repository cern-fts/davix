#include "httprequest.hpp"

namespace Davix {

HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{
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

} // namespace Davix
