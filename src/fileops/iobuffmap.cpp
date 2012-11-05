#include "iobuffmap.hpp"
#include <httprequest.hpp>

#include <cstring>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>



const std::string req_header_byte_range("Range");

namespace Davix {

void setup_offset_request(HttpRequest* req, off_t start_len, size_t size_read);
ssize_t read_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err);


IOBuffMap::IOBuffMap(Context & c, const Uri & uri, const RequestParams & params) : _c(c), _uri(uri), _params(params)
{
    _req=NULL;
    _pos =0;
    _file_size = 0;
}

IOBuffMap::~IOBuffMap(){
    delete _req;
}

bool IOBuffMap::open(int flags, DavixError **err){
    DavixError* tmp_err=NULL;
    bool res = false;
    if(checkIsOpen(NULL))
        return true;
    // execute a HEAD request for testing verifying file presence and staging
    std::auto_ptr<HttpRequest> req( _c.createRequest(_uri, &tmp_err));
    if(req.get() != NULL){
        req->set_parameters(_params);
        req->setRequestMethod("HEAD");
        if( req->executeRequest(&tmp_err) == 0){
            if( (res = httpcodeIsValid(req->getRequestCode())) == false
                    && !( ( flags & O_RDONLY) && req->getRequestCode() == 404) ){
                httpcodeToDavixCode(req->getRequestCode(),davix_scope_http_request(),", while open", &tmp_err);
            }else{
                DavixError::clearError(&tmp_err);
            }
        }

    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return res;
}

ssize_t IOBuffMap::read(void *buf, size_t count, DavixError **err){
    DppLocker l(_rwlock);
    return -1;
}

ssize_t IOBuffMap::write(const void *buf, size_t count, DavixError **err){
    DppLocker l(_rwlock);
    return -1;
}

off_t IOBuffMap::lseek(off_t offset, int flags, DavixError **err){
    DppLocker l(_rwlock);
    switch(flags){
        case SEEK_CUR:
            _pos += offset;
            break;
        case SEEK_END:
            _pos = _file_size += offset;
            break;
        default:
            _pos = offset;
            break;

    }
    return _pos;
}

ssize_t IOBuffMap::putOps(const void *buf, size_t count, off_t offset, DavixError **err){
    return -1;
}

ssize_t IOBuffMap::getOps(void *buf, size_t count, off_t offset, DavixError **err){
    DavixError * tmp_err=NULL;
    ssize_t ret = -1;
    davix_log_debug(" -> getOps operation for %s with size %ld and offset %ld",_uri.getString().c_str(), count, offset);

    std::auto_ptr<HttpRequest> req( _c.createRequest(_uri, &tmp_err));
    if(req.get() != NULL){
        req->set_parameters(_params);
        setup_offset_request(req.get(), offset, offset+count);
        if(req->beginRequest(&tmp_err) ==0){
            ret = read_segment_request(req.get(), buf, count, offset, &tmp_err);
            if(offset != 0 && req->getRequestCode() != 206){
                davix_log_debug(" WARNING : server does not support Request with range ! : %s ",_uri.getString().c_str());
            }
        }
        req->endRequest(NULL);
    }
    davix_log_debug(" end getOps operation for %s <- ",_uri.getString().c_str());
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}



bool IOBuffMap::checkIsOpen(DavixError **err){
    if(_req == NULL){
        DavixError::setupError(err, davix_scope_http_request(), StatusCode::OperationNonSupported, "File not open, Error");
        return false;
    }
    return true;
}


void setup_offset_request(HttpRequest* req, off_t start_len, size_t size_read){
    std::string offset_value("bytes=");

    if(start_len > 0){
        const size_t s_buff = 20+log10(std::max<size_t>(start_len, size_read)+1)*2; //calculate the buffer size for the string val
        char buffer[s_buff]; // calc buffer size

        if(size_read > 0)
            snprintf(buffer, s_buff, "bytes=%ld-%ld", (unsigned long) start_len, (unsigned long) size_read);
        else
            snprintf(buffer, s_buff, "bytes=%ld-", (unsigned long) start_len);
        req->addHeaderField(req_header_byte_range, buffer);
    }


}

ssize_t read_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err){
    DavixError* tmp_err=NULL;
    ssize_t ret, tmp_ret;
    char* p_buff =(char*) buffer;
    size_t s_read= size_read;
    ret = tmp_ret = 0;

    do{
        tmp_ret= req->readBlock(p_buff, s_read, &tmp_err);
        if(tmp_ret > 0){ // tmp_ret bytes readed
            ret += tmp_ret;
        }
        if(ret > 0 && ret < size_read){
            p_buff+= tmp_ret;
            s_read -= tmp_ret;
        }
    }while( tmp_ret > 0
            &&  ret < size_read);

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return ret;
}






} // namespace Davix
