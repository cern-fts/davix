#include "iobuffmap.hpp"
#include <httprequest.hpp>
#include <posix/davposix.hpp>

#include <sstream>
#include <string>

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



void setup_offset_request(HttpRequest* req, off_t start_len, size_t size_read){
    std::string offset_value("bytes=");

    if(start_len > 0 || size_read >0 ){
        const size_t s_buff = 20+(size_t)log10(std::max<size_t>(start_len, size_read)+1)*2; //calculate the buffer size for the string val
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

     while(ret < begin_offset && !tmp_err){
         if( (ret + size_read) < begin_offset) // use buffer like trash for useless content
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
         ret = -1;
     }
     return ret;
}

int get_valid_cache_file(FILE** stream, DavixError** err){
    if(stream == NULL){
        DavixError::setupError(err, davix_scope_io_cache(), StatusCode::InvalidFileHandle, "Invalid file stream");
        return -1;
    }
    if( *stream == NULL){
        if( ( *stream =  tmpfile() ) == NULL){
            std::ostringstream ss;
            ss << "Error while file-cache creation: " << strerror(errno) << std::endl;
            DavixError::setupError(err, davix_scope_io_cache(), StatusCode::SystemError, ss.str().c_str());
            return -1;
        }

    }
    return 0;
}

///////////////////////
///////////////////////
///////////////////////

HttpIO::HttpIO(Context &c, const Uri &uri, const RequestParams *params) :
    _c(c),
    _uri(uri),
    _params(params),
    _rwlock(),
    _read_pos(0),
    _read_endfile(false),
    _read_req(NULL)
{

}

HttpIO::~HttpIO(){
    delete _read_req;
}

ssize_t HttpIO::readFullBuff(void *buffer, size_t size_read, DavixError **err){
    ssize_t ret = -1;
    DavixError * tmp_err=NULL;

    if(_read_endfile)
        return 0;

    if( _read_req == NULL
            && (_read_req = _c.createRequest(_uri, &tmp_err)) != NULL){
        _read_req->setParameters(_params);
        if(_read_req->beginRequest(&tmp_err) ==0
            && (_read_req->getRequestCode() != 200)){
                httpcodeToDavixCode(_read_req->getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                delete _read_req;
                _read_req = NULL;

        }
    }

    if(_read_req != NULL){ // valid request -> proceed to read
        ret = read_segment_request(_read_req, buffer, size_read, _read_pos, &tmp_err);
        if(ret > 0){
            _read_pos += ret;
            if(ret < (ssize_t) size_read){ // end of file
                _read_endfile =true;
                _read_req->endRequest(NULL);
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


ssize_t HttpIO::readPartialBuffer(void *buf, size_t count, off_t offset, DavixError **err){
    DavixError * tmp_err=NULL;
    ssize_t ret = -1;
    DAVIX_DEBUG(" -> getOps operation for %s with size %ld and offset %ld",_uri.getString().c_str(), count, offset);
    if(count ==0)
        return 0;

    std::auto_ptr<HttpRequest> req( _c.createRequest(_uri, &tmp_err));
    if(req.get() != NULL){
        req->setParameters(_params);
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
    DAVIX_DEBUG(" end getOps operation for %s <- ",_uri.getString().c_str());
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

ssize_t HttpIO::putFullFromFD(const void *buf, size_t count, off_t offset, DavixError **err){
    return -1;
}


int HttpIO::stat(struct stat *st, DavixError **err){
    RequestParams p(_params);
    p.setProtocol(DAVIX_PROTOCOL_HTTP);

    DavPosix posix(&_c);
    return posix.stat(&p, _uri.getString(), st, err);
}

void HttpIO::resetFullRead(){
    if(_read_req)
        delete _read_req;
    _read_pos =0;
}

/////////////////////////////////////////////////////
////////////////////////////////////////////////////////

HttpIOBuffer::HttpIOBuffer(Context &c, const Uri &uri, const RequestParams *params) :
    HttpIO(c, uri, params),
    _file_size(0),
    _file_exist(false),
    _pos(false),
    _opened(false)
{

}

HttpIOBuffer::~HttpIOBuffer(){

}

bool HttpIOBuffer::open(int flags, DavixError **err){
    DavixError* tmp_err=NULL;
    bool res = false;
    if(_opened)
        return true;
    // configure local cache
    /*if( (flags & O_WRONLY) || (flags & O_RDWR)){
        if(get_valid_cache_file(&_cache_stream, &tmp_err) !=0)
            return -1;
    }*/

    struct stat st;
    if( stat(&st, &tmp_err) ==0){
        if( (flags & O_EXCL) && ( flags & O_CREAT)){
            DavixError::setupError(&tmp_err, davix_scope_io_cache(),
                                   StatusCode::FileExist, "file exist and O_EXCL flag usedin open");
        }else{
            _file_size = st.st_size;
            _file_exist = true;
            _opened = true;
        }
    }else if (tmp_err->getStatus() == StatusCode::FileNotFound
              &&  (flags & O_CREAT)
              && ((flags & O_RDWR) || (flags  & O_WRONLY))){
        DavixError::clearError(&tmp_err);
        _file_size = 0;
        _file_exist = false;
        _opened = true;

    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return res;
}

ssize_t HttpIOBuffer::read(void *buf, size_t count, DavixError **err){
    DppLocker l(_rwlock);
    DavixError* tmp_err = NULL;
    ssize_t ret =-1;

    if(_pos ==0) // reset read ahead offset to default if try to read a full file
        resetFullRead();
    if(_pos == _read_pos){
        // try read ahead strategie
        ret = readFullBuff(buf, count, &tmp_err);
    }else{ // fallback on partial read
        ret = readPartialBuffer(buf, count, _pos, &tmp_err);
    }
    if(ret > 0)
        _pos += ret;

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


ssize_t HttpIOBuffer::pread(void *buf, size_t count, off_t offset, DavixError **err){
    DppLocker l(_rwlock);
    DavixError* tmp_err = NULL;
    ssize_t ret = readPartialBuffer(buf, count, offset, &tmp_err);

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


off_t HttpIOBuffer::lseek(off_t offset, int flags, DavixError **err){
    DppLocker l(_rwlock);
    switch(flags){
        case SEEK_CUR:
            _pos += offset;
            break;
        case SEEK_END:
            _pos = ( _file_size += offset);
            break;
        case SEEK_SET:
        default:
            _pos = offset;
            break;

    }
    return _pos;
}

ssize_t HttpIOBuffer::write(const void *buf, size_t count, DavixError **err){
    DppLocker l(_rwlock);
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if(_pos != 0){
        DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::OperationNonSupported, " Multi-part write is not supported by Http !");
    }else{
        std::auto_ptr<HttpRequest> req( _c.createRequest(_uri, &tmp_err));
        if(req.get() != NULL){
            req->setRequestMethod("PUT");
            req->setParameters(_params);
            req->setRequestBodyBuffer(buf, count);
            if(req->beginRequest(&tmp_err) ==0){
                if(req->getRequestCode() != 200 ||  req->getRequestCode() != 204){ // out of file, end of file
                    ret = count; // end of file
                    _pos += count;
                }else{
                    httpcodeToDavixCode(req->getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                    ret = -1;
                }
            }
            req->endRequest(NULL);
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

ssize_t HttpIOBuffer::pwrite(const void *buf, size_t count, off_t offset, DavixError **err){
    return -1;
}


} // namespace Davix
