#include "davix_rw.h"
#include <davix_types.h>
#include <posix/davposix.hpp>
#include <fileops/iobuffmap.hpp>


struct Davix_fd{

    Davix_fd(Davix::HttpIOBuffer * buff) : io_handler(buff){}
    std::auto_ptr<Davix::HttpIOBuffer> io_handler;
};

namespace Davix{


int davix_check_rw_fd(DAVIX_FD* fd, DavixError** err){
    if(fd == NULL){
        DavixError::setupError(err, davix_scope_http_request(),StatusCode::InvalidFileHandle, "Invalid Davix file descriptor");
        return -1;
    }
    return 0;
}


DAVIX_FD* DavPosix::open(const RequestParams * _params, const std::string & url, int flags, DavixError** err){
    davix_log_debug(" -> davix_open");
    DavixError* tmp_err=NULL;
    Davix_fd* fd = NULL;
    Uri uri(url);

    if(uri.getStatus() == StatusCode::OK){
        fd = new Davix_fd( new HttpIOBuffer(*context, uri, _params));
        fd->io_handler->open( flags, &tmp_err);
    }else{
        DavixError::setupError(&tmp_err, davix_scope_http_request(), uri.getStatus(), " Uri invalid in Davix::Open");
    }

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        delete fd;
        fd= NULL;
    }
    davix_log_debug(" davix_open <-");
    return fd;
}


ssize_t DavPosix::read(DAVIX_FD* fd, void* buf, size_t count, Davix::DavixError** err){
    davix_log_debug(" -> davix_read");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = fd->io_handler->read(buf, count, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    davix_log_debug(" davix_read <-");
    return ret;
}


ssize_t DavPosix::write(DAVIX_FD* fd, const void* buf, size_t count, Davix::DavixError** err){
    davix_log_debug(" -> davix_write");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = fd->io_handler->write(buf, count, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    davix_log_debug(" davix_write <-");
    return ret;
}



off_t DavPosix::lseek(DAVIX_FD* fd, off_t offset, int flags, Davix::DavixError** err){
    davix_log_debug(" -> davix_lseek");
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if( davix_check_rw_fd(fd, &tmp_err) ==0){
        ret = fd->io_handler->lseek(offset, flags, &tmp_err);
    }


    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    davix_log_debug(" davix_lseek <-");
    return ret;
}


int DavPosix::close(DAVIX_FD* fd, Davix::DavixError** err){
    if(fd){
        delete fd;
    }
    return 0;
}



}
