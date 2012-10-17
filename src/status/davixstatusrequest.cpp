#include "davixstatusrequest.hpp"
#include <davix_types.h>

namespace Davix {


struct DavixErrorInternal{
    DavixErrorInternal(const std::string &scope, StatusCode::Code errCode, const std::string &errMsg){
        this->scope = scope;
        this->errMsg = errMsg;
        this->code = errCode;
    }

    std::string scope;
    StatusCode::Code code;
    std::string errMsg;
};


DavixError::DavixError(const std::string &scope, StatusCode::Code errCode, const std::string &errMsg){
    d_ptr = new DavixErrorInternal(scope, errCode, errMsg);
}


DavixError::~DavixError(){
    delete d_ptr;
}


const std::string & DavixError::getErrMsg() const{
    return d_ptr->errMsg;
}

void DavixError::setErrMsg(const std::string &msg){
    d_ptr->errMsg = msg;
}

void DavixError::setStatus(const StatusCode::Code c){
    d_ptr->code = c;
}

StatusCode::Code DavixError::getStatus() const{
    return d_ptr->code;
}


void DavixError::setupError(DavixError **err, const std::string &scope, StatusCode::Code errCode, const std::string &errMsg){
    if(err){
        if(*err){
            // error msg
        }
        *err = new DavixError(scope, errCode, errMsg);
    }
}


} // namespace Davix
