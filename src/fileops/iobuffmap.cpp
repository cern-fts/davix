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
ssize_t read_truncated_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err);


IOBuffMap::IOBuffMap(Context & c, const Uri & uri, const RequestParams * params) : _c(c), _uri(uri), _params(params)
{
    _read_req=NULL;
    _pos =0;
    _read_pos =0;
    _file_size = 0;
    _read_endfile = false;
    _opened = false;
}

IOBuffMap::~IOBuffMap(){
    delete _read_req;
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
                _opened = true;
            }
        }

    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return res;
}

ssize_t IOBuffMap::read(void *buf, size_t count, DavixError **err){
    DppLocker l(_rwlock);
    DavixError* tmp_err = NULL;
    ssize_t ret =-1;

    // try read ahead strategie
    ret = readAheadRequest(buf, count, &tmp_err);
    if( ret  <0 && !tmp_err){ // fallback on partial read
       ret = getOps(buf, count, _pos, &tmp_err);
    }
    if(ret > 0)
        _pos += ret;

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
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
    if(count ==0)
        return 0;

    std::auto_ptr<HttpRequest> req( _c.createRequest(_uri, &tmp_err));
    if(req.get() != NULL){
        req->set_parameters(_params);
        setup_offset_request(req.get(), offset, offset+count);
        if(req->beginRequest(&tmp_err) ==0){
            if(req->getRequestCode() == 416 ){ // out of file, end of file
                ret = 0; // end of file
            }else{
                if(req->getRequestCode() == 206 ){ // partial request supported, just read !
                    ret = read_segment_request(req.get(), buf, count, offset, &tmp_err);
                }else if( req->getRequestCode() == 200){ // full request content -> skip useless content
                    ret = read_truncated_segment_request(req.get(), buf, count, offset, &tmp_err);
                }else{
                    httpcodeToDavixCode(req->getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                }
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
    return _opened;
}


ssize_t IOBuffMap::readAheadRequest(void * buffer, size_t size_read, DavixError ** err){
    ssize_t ret = -1;
    DavixError * tmp_err=NULL;

    if(_pos ==0){ // reset read ahead offset to default if try to read a full file
        _read_pos =0;
        _read_endfile = false;
    }

    if(_pos == _read_pos ){ // continue a already started readding
        davix_log_debug(" -> try readaheal from %ld of size %lfd", _read_pos, size_read);

        if(_read_endfile)
            return 0;

        if( _read_req == NULL){
            _read_req = _c.createRequest(_uri, &tmp_err);
            if(_read_req != NULL){
                _read_req->set_parameters(_params);
                if(_read_req->beginRequest(&tmp_err) ==0){
                    if(_read_req->getRequestCode() != 200 ){
                        httpcodeToDavixCode(_read_req->getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                        delete _read_req;
                        _read_req = NULL;
                    }
                }
            }
        }

        if(_read_req != NULL){ // valid request -> proceed to read
            ret = read_segment_request(_read_req, buffer, size_read, _read_pos, &tmp_err);
            if(ret > 0){
                _read_pos += ret;
                if(ret < size_read) // end of file
                    _read_endfile =true;
            }
        }
    }

    if((_read_endfile || ret < 0) && _read_req){
        delete _read_req;
        _read_req = NULL;
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


void setup_offset_request(HttpRequest* req, off_t start_len, size_t size_read){
    std::string offset_value("bytes=");

    if(start_len > 0 || size_read >0 ){
        const size_t s_buff = 20+log10(std::max<size_t>(start_len, size_read)+1)*2; //calculate the buffer size for the string val
        char buffer[s_buff]; // calc buffer size

        if(size_read > 0)
            snprintf(buffer, s_buff, "bytes=%ld-%ld", (unsigned long) start_len, (unsigned long) start_len+size_read);
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
        if(ret > 0 && ret < (ssize_t) size_read){
            p_buff+= tmp_ret;
            s_read -= tmp_ret;
        }
    }while( tmp_ret > 0
            &&  ret < (ssize_t) size_read);

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return ret;
}


ssize_t read_truncated_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err){
     DavixError* tmp_err=NULL;
     ssize_t ret=0, tmp_ret=0;
     const ssize_t begin_offset = (ssize_t) off_set;
     char * p_buffer = (char*) buffer;

     while(ret < begin_offset && !tmp_err){ // use buffer like trash for useless content
         if( (ret + size_read) < begin_offset)
            tmp_ret = req->readBlock(p_buffer, size_read, &tmp_err);
         else
            tmp_ret = req->readBlock(p_buffer, begin_offset - ret, &tmp_err);

         if(tmp_ret == 0)
             return 0;

        ret += tmp_ret;
     }

     if(!tmp_err){
        ret = read_segment_request(req, p_buffer, size_read,  off_set, &tmp_err);
     }

     if(tmp_err){
         DavixError::propagateError(err, tmp_err);
         return -1;
     }
     return ret;
}




} // namespace Davix
