#include <davix_internal.hpp>

#include <neon/neonrequest.hpp>
#include <davix_context_internal.hpp>
#include <request/httprequest.hpp>

namespace Davix {

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

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

void HttpRequest::setRequestBody(const std::string &body){
    d_ptr->setRequestBody(body);
}

void HttpRequest::setRequestBody(const void *buffer, dav_size_t len_buff){
    d_ptr->setRequestBody(buffer, len_buff);
}

void HttpRequest::setRequestBody(int fd, dav_off_t offset, dav_size_t len){
    d_ptr->setRequestBody(fd, offset, len);
}

void HttpRequest::setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata){
    d_ptr->setRequestBody(provider, len, udata);
}

int HttpRequest::getRequestCode(){
    return d_ptr->getRequestCode();
}

int HttpRequest::executeRequest(DavixError **err){
    TRY_DAVIX{
        return d_ptr->executeRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}

int HttpRequest::beginRequest(DavixError **err){
    TRY_DAVIX{
        return d_ptr->beginRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}

dav_ssize_t HttpRequest::readBlock(char *buffer, dav_size_t max_size, DavixError **err){
    TRY_DAVIX{
        return d_ptr->readBlock(buffer, max_size, err);
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
    return d_ptr->readSegment(buffer, max_size, err);
}

dav_ssize_t HttpRequest::readLine(char *buffer, dav_size_t max_size, DavixError **err){
    DAVIX_DEBUG("Davix::Request::readLine want to read a line of max %lld chars", max_size);
    dav_ssize_t ret= -1;
    TRY_DAVIX{
        if( (ret =  d_ptr->readLine(buffer, max_size, err)) >= 0){
            DAVIX_DEBUG("Davix::Request::readLine got %lld chars", ret);
            DAVIX_TRACE("Davix::Request::readLine content\n{{%.*s}}\n", ret, buffer);
        }
    }CATCH_DAVIX(err)
    return ret;
}

void HttpRequest::discardBody(DavixError** err){
    char buffer[1024];
    dav_ssize_t read;
    TRY_DAVIX{
        do {
            read = d_ptr->readSegment(buffer, sizeof(buffer), err);
        } while (read > 0 && *err == NULL);
    }CATCH_DAVIX(err)
}

dav_ssize_t HttpRequest::readToFd(int fd, DavixError** err){
    TRY_DAVIX{
        return d_ptr->readToFd(fd, 0, err);
    }CATCH_DAVIX(err)
    return -1;
}


dav_ssize_t HttpRequest::readToFd(int fd, dav_size_t read_size, DavixError** err){
    TRY_DAVIX{
        return d_ptr->readToFd(fd, read_size, err);
    }CATCH_DAVIX(err)
    return -1;
}

int HttpRequest::endRequest(DavixError **err){
    TRY_DAVIX{
        return d_ptr->endRequest(err);
    }CATCH_DAVIX(err)
    return -1;
}


void HttpRequest::clearAnswerContent(){
    d_ptr->clearAnswerContent();
}

bool HttpRequest::getAnswerHeader(const std::string &header_name, std::string &value) const{
    return d_ptr->getAnswerHeader(header_name, value);
}

size_t HttpRequest::getAnswerHeaders( HeaderVec & vec_headers) const{
    return d_ptr->getAnswerHeaders(vec_headers);
}

const char* HttpRequest::getAnswerContent(){
    return d_ptr->getAnswerContent();
}

/// get content length
dav_ssize_t HttpRequest::getAnswerSize() const{
    return d_ptr->getAnswerSize();
}

HttpCacheToken* HttpRequest::extractCacheToken()const{
    return NULL;
}

void HttpRequest::useCacheToken(const HttpCacheToken *token){
}

/// set a HttpRequest flag
void HttpRequest::setFlag(const RequestFlag::RequestFlag flag, bool value){
    if(value)
        d_ptr->_req_flag |=  flag;
    else
        d_ptr->_req_flag &= ~(flag);
}

/// get a HttpRequest flag value
bool HttpRequest::getFlag(const RequestFlag::RequestFlag flag){
    return d_ptr->_req_flag & ((int) flag);
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



void httpcodeToDavixCode(int code, const std::string & scope, const std::string & end_message, DavixError** err){
    StatusCode::Code dav_code;
    std::string str_msg;
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
        case 409:
            dav_code = StatusCode::FileExist;
            str_msg = "HTTP Conflict";
            break;
        case 423:           /* Locked */
        case 403:           /* Forbidden */
            dav_code = StatusCode::PermissionRefused;
            str_msg = "HTTP Permission refused";
            break;
        break;
        case 400:           /* Bad Request */
        case 405:           /* Method Not Allowed */
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
