#include <config.h>
#include <davix_types.h>
#include "davixstatusrequest.hpp"



namespace Davix {



struct DavixErrorInternal{
    DavixErrorInternal(const std::string &scope, StatusCode::Code errCode, const std::string &errMsg) :
        _scope(scope),
        _code(errCode),
        _errMsg(errMsg){
    }

    DavixErrorInternal(const DavixErrorInternal & e) :
        _scope(e._scope),
        _code(e._code),
        _errMsg(e._errMsg){

    }

    std::string _scope;
    StatusCode::Code _code;
    std::string _errMsg;
};


DavixError::DavixError(const std::string &scope, StatusCode::Code errCode, const std::string &errMsg) :
    d_ptr(new DavixErrorInternal(scope, errCode, errMsg)){

}

DavixError::DavixError(const DavixError & e) :
    d_ptr(new DavixErrorInternal(*(e.d_ptr))){

}

DavixError & DavixError::operator =(const DavixError & e){
    if(d_ptr)
        delete d_ptr;
    d_ptr = new DavixErrorInternal(*(e.d_ptr));
    return *this;
}


DavixError::~DavixError(){
    delete d_ptr;
}

DavixError* DavixError::clone(){
    return new DavixError(*this);
}


const std::string & DavixError::getErrMsg() const{
    return d_ptr->_errMsg;
}

void DavixError::setErrMsg(const std::string &msg){
    d_ptr->_errMsg = msg;
}

void DavixError::setStatus(const StatusCode::Code c){
    d_ptr->_code = c;
}

StatusCode::Code DavixError::getStatus() const{
    return d_ptr->_code;
}

const std::string & DavixError::getErrScope() const{
    return d_ptr->_scope;
}

void DavixError::setErrScope(const std::string &scope){
    d_ptr->_scope = scope;
}



void DavixError::setupError(DavixError **err, const std::string &scope, StatusCode::Code errCode, const std::string &errMsg){
    if(err){
        if(*err){
            // error msg
        }
        *err = new DavixError(scope, errCode, errMsg);
    }
}


void DavixError::propagateError(DavixError **newErr, DavixError *oldErr){
    propagatePrefixedError(newErr, oldErr, "");
}

void DavixError::propagatePrefixedError(DavixError **newErr, DavixError *oldErr, const std::string &prefix){
    if(newErr){
        if(*newErr != NULL){
            std::cerr << "***ERROR*** in propagateError, *newErr is not NULL impossible to overwrite ... "
                     " old error wass" << ((oldErr)?(oldErr->getErrMsg()):"<NULL>") << std::endl;
        }else{
            *newErr = oldErr;
            if(*newErr && prefix.empty() == false){
                std::string new_mess(prefix);
                (*newErr)->d_ptr->_errMsg = new_mess.append((*newErr)->d_ptr->_errMsg);
            }
        }
    }

}

void DavixError::clearError(DavixError **err){
    if(err && *err){
        delete *err;
        *err = NULL;
    }
}

void davix_errno_to_davix_error(int errcode, const std::string & scope, const std::string & msg, DavixError** newErr){
    StatusCode::Code c;
    std::string msg_final;
    switch(errcode){
        case 0:
            return;
        case ENOENT:
            c = StatusCode::FileNotFound;
            msg_final = "File not found " + msg;
            break;
        case EPERM:
        case EACCES:
            c = StatusCode::PermissionRefused;
            msg_final = "Permission refused " + msg;
            break;
        case EISDIR:
            c = StatusCode::IsADirectory;
            msg_final = "is a directory " + msg;
            break;
        case EINVAL:
            c = StatusCode::InvalidArgument;
            msg_final = "invalid argument " + msg;
            break;
        case EIO:
            c = StatusCode::InvalidArgument;
            msg_final = "Input/output error" + msg;
            break;
        case ENOTDIR:
            c = StatusCode::IsNotADirectory;
            msg_final = "is not a directory" + msg;
            break;
        default:
            c = StatusCode::SystemError;
            msg_final = std::string(strerror(errcode)) +  " " + msg;
    }
    DavixError::setupError(newErr, scope, c, msg_final);
}

std::string davix_scope_stat_str(){
    return "Davix::Posix::stat";
}

std::string davix_scope_mkdir_str(){
    return "Davix::Posix::mkdir";
}

std::string davix_scope_directory_listing_str(){
    return "Davix::Posix::listdir";
}

std::string davix_scope_http_request(){
    return "Davix::HttpRequest";
}

std::string davix_scope_xml_parser(){
    return "Davix::XMLParser";
}

std::string davix_scope_uri_parser(){
    return "Davix::Uri::Parser";
}

std::string davix_scope_davOps_str(){
    return "Davix::Dav::Ops";
}

std::string davix_scope_io_cache(){
    return "Davix::IO:Cache";
}

std::string davix_scope_x509cred(){
    return "Davix::X509cred";
}

} // namespace Davix

DAVIX_C_DECL_BEGIN

using namespace Davix;

///  @brief clear a davix error object and release its memory, and set the error pointer to NULL
///
void davix_error_clear(davix_error_t* ptr_err){
    DavixError::clearError((DavixError**) ptr_err);
}

/// @brief create a new davix error object
///
void davix_error_setup(davix_error_t* ptr_err, const char* scope, int status_code, const char* msg){
    DavixError::setupError((DavixError**) ptr_err, scope, (Davix::StatusCode::Code)status_code, msg);
}


const char* davix_error_msg(davix_error_t err){
    return ((DavixError*) err)->getErrMsg().c_str();
}

int davix_error_code(davix_error_t err){
    return ((DavixError*) err)->getStatus();
}

const char* davix_error_scope(davix_error_t err){
    return NULL; // TODO
}

void davix_error_propagate(davix_error_t* newErr, davix_error_t oldErr ){
    DavixError::propagateError((DavixError**)newErr, (DavixError*) oldErr);
}


DAVIX_C_DECL_END


