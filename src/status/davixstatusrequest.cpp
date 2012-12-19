#include "davixstatusrequest.hpp"
#include <davix_types.h>

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

std::string davix_scope_stat_str(){
    return "[davix_stat]";
}

std::string davix_scope_mkdir_str(){
    return "[davix_mkdir]";
}

std::string davix_scope_directory_listing_str(){
    return "[davix_directory_listing]";
}

std::string davix_scope_http_request(){
    return "[davix_http_request]";
}

std::string davix_scope_xml_parser(){
    return "[davix_xml_parser]";
}

std::string davix_scope_uri_parser(){
    return "[davix_uri_parser]";
}

std::string davix_scope_davOps_str(){
    return "[davix_dav_operation]";
}

std::string davix_scope_io_cache(){
    return "[davix_io_cache]";
}

std::string davix_scope_x509cred(){
    return "[davix_x509cred]";
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


