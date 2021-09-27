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
#include <utils/davix_logger_internal.hpp>
#include <utils/davix_types.hpp>
#include <status/davixstatusrequest.hpp>
#include <status/DavixErrorInternal.hpp>

namespace Davix {

DavixError::DavixError(const std::string &scope, StatusCode::Code errCode, const std::string &errMsg) :
    d_ptr(new DavixErrorInternal(scope, errCode, errMsg)){

}

DavixError::DavixError(const DavixError & e) :
    d_ptr(new DavixErrorInternal(*(e.d_ptr))){

}

DavixError & DavixError::operator =(const DavixError & e){
    DavixError tmp(e);
    this->swap(tmp);
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
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Error Override of previous DavixError, BUG !");
            // error msg
        }
        *err = new DavixError(scope, errCode, errMsg);
    }
}


void DavixError::propagateError(DavixError **newErr, DavixError *oldErr){
    propagatePrefixedError(newErr, oldErr, "");
}

void DavixError::propagatePrefixedError(DavixError **newErr, DavixError *oldErr, const std::string &prefix){
    if(oldErr== NULL)
        return;

    if(newErr){
        if(*newErr != NULL){
            std::cerr << "***ERROR*** in propagateError, *newErr is not NULL impossible to overwrite ... "
                     " old error was" << oldErr->getErrMsg()<< std::endl;
        }else{
            *newErr = oldErr;
            if(*newErr && prefix.empty() == false){
                std::string new_mess(prefix);
                new_mess.append(" ");
                (*newErr)->d_ptr->_errMsg = new_mess.append((*newErr)->d_ptr->_errMsg);
            }
        }
    }

}

void DavixError::swap(DavixError &err){
    std::swap(d_ptr, err.d_ptr);
}

void DavixError::clearError(DavixError **err){
    if(err && *err){
        delete *err;
        *err = NULL;
    }
}


void errno_to_davix_exception(int errno_code, const std::string &scope, const std::string &msg){
    DavixError* tmp_err=NULL;
    davix_errno_to_davix_error(errno_code, scope, msg, &tmp_err);
    checkDavixError(&tmp_err);
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


// extension
struct DavixException::DavixExceptionIntern{

};


DavixException::DavixException(const std::string &scope, StatusCode::Code c, const std::string &msg) throw() :
    std::exception(),
    e(scope, c, msg),
    d_ptr(NULL){

}

DavixException::DavixException(const DavixException &orig) throw() : e(orig.e), d_ptr(orig.d_ptr)
{

}

DavixException::DavixException(DavixError **err) :
std::exception(),
  e( "Davix::Error", StatusCode::UnknownError, "Error, no valid DavixError triggered"),
d_ptr(NULL){
    if(err != NULL && *err != NULL){
        e.swap(**err);
        DavixError::clearError(err);
    }
}


void checkDavixError(DavixError **err){
    if(err && *err){
        throw DavixException(err);
    }
}


DavixException::~DavixException() throw(){
    delete d_ptr;
}

const char* DavixException::what() const throw() {
    return e.getErrMsg().c_str();
}

StatusCode::Code DavixException::code() const throw(){
    return e.getStatus();
}

const char* DavixException::scope() const throw(){
    return e.getErrScope().c_str();
}

void DavixException::toDavixError(DavixError **err){
    DavixError::propagateError(err, new DavixError(this->e));
}


std::string davix_scope_stat_str(){
    return "Davix::stat";
}

std::string davix_scope_mkdir_str(){
    return "Davix::mkdir";
}

std::string davix_scope_rm_str(){
    return "Davix::rm";
}

std::string davix_scope_mv_str(){
    return "Davix::mv";
}

std::string davix_scope_directory_listing_str(){
    return "Davix::Posix::listdir";
}

std::string davix_scope_http_request(){
    return "Davix::HttpRequest";
}

std::string davix_scope_meta(){
    return "Davix::MetaDataOps";
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

std::string davix_scope_io_buff(){
    return "Davix::IO:Buff";
}

std::string davix_scope_x509cred(){
    return "Davix::X509cred";
}



} // namespace Davix



