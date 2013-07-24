#include <config.h>
#include "iobuffmap.hpp"

#include <davix_types.h>
#include <request/httprequest.hpp>
#include <logger/davix_logger_internal.h>
#include <http_util/http_util.hpp>
#include <fileops/httpiovec.hpp>
#include <fileops/davmeta.hpp>


#include <sstream>
#include <string>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>






namespace Davix {

dav_ssize_t read_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read,  dav_off_t off_set, DavixError**err);
dav_ssize_t read_truncated_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read,  dav_off_t off_set, DavixError**err);




dav_ssize_t read_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read,  dav_off_t off_set, DavixError**err){
    DavixError* tmp_err=NULL;
    dav_ssize_t ret, tmp_ret;
    char* p_buff =(char*) buffer;
    dav_size_t s_read= size_read;
    ret = tmp_ret = 0;

    DAVIX_TRACE("Davix::IOMap::readSegment: want to read %lld bytes ", size_read);
    do{
        tmp_ret= req->readBlock(p_buff, s_read, &tmp_err);
        if(tmp_ret > 0){ // tmp_ret bytes readed
            ret += tmp_ret;
        }
        if(ret > 0 && ret < (dav_ssize_t) size_read){
            p_buff+= tmp_ret;
            s_read -= tmp_ret;
        }
    }while( tmp_ret > 0
            &&  ret < (dav_ssize_t) size_read);

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    DAVIX_TRACE("Davix::IOMap::readSegment: got %lld bytes ", ret);
    return ret;
}


dav_ssize_t read_truncated_segment_request(HttpRequest* req, void* buffer, dav_size_t size_read,  dav_off_t off_set, DavixError**err){
     DavixError* tmp_err=NULL;
     dav_ssize_t ret=0, tmp_ret=0;
     const dav_ssize_t begin_offset = (dav_ssize_t) off_set;
     const dav_ssize_t ssize_read = size_read;
     char * p_buffer = (char*) buffer;

     while(ret < begin_offset && !tmp_err){
         if( (ret + ssize_read) < begin_offset) // use buffer like trash for useless content
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
        DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidFileHandle, "Invalid file stream");
        return -1;
    }
    if( *stream == NULL){
        if( ( *stream =  tmpfile() ) == NULL){
            std::ostringstream ss;
            ss << "Error while file-cache creation: " << strerror(errno) << std::endl;
            DavixError::setupError(err, davix_scope_io_buff(), StatusCode::SystemError, ss.str().c_str());
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
    _token(),
    _read_req(NULL)
{

}

HttpIO::~HttpIO(){
    delete _read_req;
}

dav_ssize_t HttpIO::readFullBuff(void *buffer, dav_size_t size_read, DavixError **err){
    dav_ssize_t ret = -1;
    DavixError * tmp_err=NULL;

    if(_read_endfile)
        return 0;

    if( _read_req == NULL
            && (_read_req = new HttpRequest(_c,_uri, &tmp_err)) != NULL
            && tmp_err == NULL ){
        _read_req->setParameters(_params);
        _read_req->useCacheToken(_token.get());
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
            if(ret < (dav_ssize_t) size_read){ // end of file
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


// read to dynamically allocated buffer
dav_ssize_t HttpIO::readFull(std::vector<char> & buffer, DavixError** err){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1, total=0;

    DAVIX_DEBUG(" -> readFull on vector");
    GetRequest req (_c,_uri, &tmp_err);
    if(!tmp_err){
        req.setParameters(_params);
        req.useCacheToken(_token.get());
        ret = req.beginRequest(&tmp_err);
        if(!tmp_err){
            const dav_size_t s_chunk = (req.getAnswerSize() > 0)?(req.getAnswerSize()):DAVIX_BLOCK_SIZE;
            buffer.reserve(buffer.size()+ s_chunk);

            while ( (ret= req.readBlock( buffer, s_chunk, &tmp_err)) > 0){
                total += (dav_size_t) ret;
            }
            if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
                httpcodeToDavixCode(req.getRequestCode(),davix_scope_io_buff(),"read error: ", &tmp_err);
                ret = -1;
            }
        }
    }

    DAVIX_DEBUG(" <- readFull on vector");
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return (ret>0)?total:-1;
}

// read to dynamically allocated buffer
dav_ssize_t HttpIO::readFull(std::string & str_buffer , DavixError** err){
    std::vector<char> buffer;
    dav_ssize_t s = readFull(buffer, err);
    str_buffer.assign(buffer.begin(), buffer.end());
    return s;
}

dav_ssize_t HttpIO::readPartialBuffer(void *buf, dav_size_t count, dav_off_t offset, DavixError **err){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;
    DAVIX_DEBUG(" -> getOps operation for %s with size %ld and offset %ld",_uri.getString().c_str(), count, offset);
    if(count ==0)
        return 0;

    HttpRequest req(_c, _uri, &tmp_err);
    if(tmp_err == NULL){
        req.setParameters(_params);
        req.useCacheToken(_token.get());
        setup_offset_request(&req, &offset, &count,1);
        if(req.beginRequest(&tmp_err) ==0){
            if(req.getRequestCode() == 416 ){ // out of file, end of file
                ret = 0; // end of file
            }else{
                if(req.getRequestCode() == 206 ){ // partial request supported, just read !
                    ret = read_segment_request(&req, buf, count, offset, &tmp_err);
                }else if( req.getRequestCode() == 200){ // full request content -> skip useless content
                    ret = read_truncated_segment_request(&req, buf, count, offset, &tmp_err);
                }else{
                    httpcodeToDavixCode(req.getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                }
            }
        }
        req.endRequest(NULL);
    }
    DAVIX_DEBUG(" end getOps operation for %s <- ",_uri.getString().c_str());
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}




dav_ssize_t HttpIO::readPartialBufferVec(const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      const dav_size_t count_vec, DavixError** err){
    HttpVecOps vec(_c, _uri, _params, *_token);
    return vec.readPartialBufferVec(input_vec,
                                    output_vec,
                                    count_vec,
                                    err);
}

dav_ssize_t HttpIO::readToFd(int fd, dav_size_t read_size, DavixError** err){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;

    DAVIX_DEBUG(" -> readToFd for size %ld", read_size);
    GetRequest req (_c,_uri, &tmp_err);
    if(!tmp_err){
        req.setParameters(_params);
        req.useCacheToken(_token.get());
        ret = req.beginRequest(&tmp_err);
        if(!tmp_err){
            if(httpcodeIsValid(req.getRequestCode()) == false){
                httpcodeToDavixCode(req.getRequestCode(),davix_scope_io_buff(),"read error: ", &tmp_err);
                ret = -1;
            }else{
                ret= req.readToFd(fd, read_size, &tmp_err);
            }
        }
    }


    DAVIX_DEBUG(" <- readToFd for size %ld", read_size);
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


// position independant write operation,
// similar to pwrite do not need open() before
dav_ssize_t HttpIO::writeFullFromFd(int fd, dav_size_t size, DavixError** err){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;

    DAVIX_DEBUG(" -> writeFromFd for size %ld", size);
    PutRequest req (_c,_uri, &tmp_err);
    if(!tmp_err){
        req.setParameters(_params);
        req.useCacheToken(_token.get());
        req.setRequestBody(fd,0, size);
        ret = req.executeRequest(&tmp_err);
        if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
            httpcodeToDavixCode(req.getRequestCode(), davix_scope_io_buff(),
                                "read error: ", &tmp_err);
            ret = -1;
        }
    }


    DAVIX_DEBUG(" <- writeFromFd for size %ld", size);
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}



int HttpIO::stat(struct stat *st, DavixError **err){
    return stat( st, NULL, err);
}

int HttpIO::stat(struct stat* st, HttpCacheToken** token, DavixError** err){
    RequestParams p(_params);
    if(p.getProtocol()== RequestProtocol::Auto) // default -> switch to http mode
        p.setProtocol(RequestProtocol::Http);
    return Meta::posixStat(_c, _uri, &p, st, token, err);
}

void HttpIO::resetFullRead(){
    if(_read_req){
        delete _read_req;
        _read_req = NULL;
    }
    _read_pos =0;
}



/////////////////////////////////////////////////////
////////////////////////////////////////////////////////

HttpIOBuffer::HttpIOBuffer(Context &c, const Uri &uri, const RequestParams *params) :
    HttpIO(c, uri, params),
    _file_size(0),
    _file_exist(false),
    _pos(false),
    _opened(false),
    _last_advise(AdviseAuto)
{

}

HttpIOBuffer::~HttpIOBuffer(){

}

bool HttpIOBuffer::open(int flags, DavixError **err){
    DavixError* tmp_err=NULL;
    bool res = false;
    RequestParams p(_params);
    HttpCacheToken* token = NULL;
    if(_opened)
        return true;

    struct stat st;

    p.setProtocol(RequestProtocol::Http);
    if( Meta::posixStat(_c, _uri, &p, &st, &token, &tmp_err) ==0){
        if(token)
            _token.reset(token);
        if( (flags & O_EXCL) && ( flags & O_CREAT)){
            DavixError::setupError(&tmp_err, davix_scope_io_buff(),
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
    else{
        DAVIX_TRACE("File open %s, size: %ld", _uri.getString().c_str(), _file_size);
    }
    return res;
}

dav_ssize_t HttpIOBuffer::read(void *buf, dav_size_t count, DavixError **err){
    DppLocker l(_rwlock);
    DavixError* tmp_err = NULL;
    dav_ssize_t ret =-1;

    if(_pos ==0) // reset read ahead offset to default if try to read a full file
        resetFullRead();
    if(_pos == _read_pos && isAdviseFullRead()){
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


void HttpIOBuffer::prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv){
    _last_advise = adv;
}


dav_ssize_t HttpIOBuffer::pread(void *buf, dav_size_t count, dav_off_t offset, DavixError **err){
    DppLocker l(_rwlock);
    DavixError* tmp_err = NULL;
    dav_ssize_t ret = readPartialBuffer(buf, count, offset, &tmp_err);

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

dav_ssize_t HttpIOBuffer::preadVec(const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      dav_size_t count_vec, DavixError** err){
    davix_return_val_if_fail( input_vec != NULL && output_vec != NULL,-1);
    dav_ssize_t res = -1;
    if(count_vec ==1){ // one offset read request, no need of multi part
        res= (dav_ssize_t) pread(input_vec->diov_buffer, (size_t) input_vec->diov_size,  input_vec->diov_offset, err);
        output_vec->diov_buffer = input_vec->diov_buffer;
        output_vec->diov_size= res;

    }else{  // setup multi part transfer
        res = readPartialBufferVec(input_vec, output_vec, count_vec, err);
    }
    return res;
}


dav_off_t HttpIOBuffer::lseek(dav_off_t offset, int flags, DavixError **err){
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

dav_ssize_t HttpIOBuffer::write(const void *buf, dav_size_t count, DavixError **err){
    DppLocker l(_rwlock);
    dav_ssize_t ret =-1;
    DavixError* tmp_err=NULL;

    if(_pos != 0){
        DavixError::setupError(&tmp_err, davix_scope_http_request(), StatusCode::OperationNonSupported, " Multi-part write is not supported by Http !");
    }else{
        PutRequest req( _c, _uri, &tmp_err);
        if(tmp_err == NULL){
            req.setParameters(_params);
            req.setRequestBody(buf, count);
            if(req.beginRequest(&tmp_err) ==0){
                if(req.getRequestCode() != 200 ||  req.getRequestCode() != 204){ // out of file, end of file
                    ret = count; // end of file
                    _pos += count;
                }else{
                    httpcodeToDavixCode(req.getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                    ret = -1;
                }
            }
            req.endRequest(NULL);
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}

dav_ssize_t HttpIOBuffer::pwrite(const void *buf, dav_size_t count, dav_off_t offset, DavixError **err){
    return -1;
}




} // namespace Davix
