/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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

#include <config.h>
#include "iobuffmap.hpp"

#include <utils/davix_types.hpp>
#include <request/httprequest.hpp>
#include <utils/davix_logger_internal.hpp>
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

HttpIO::HttpIO()
{

}

HttpIO::~HttpIO(){
}


// read to dynamically allocated buffer
dav_ssize_t HttpIO::readFull(IOChainContext & iocontext, std::vector<char> & buffer){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1, total=0;

    DAVIX_DEBUG(" -> readFull on vector");
    GetRequest req (iocontext._context, iocontext._uri, &tmp_err);
    if(!tmp_err){
        RequestParams params(iocontext._reqparams);
        req.setParameters(params);
        ret = req.beginRequest(&tmp_err);
        if(!tmp_err){
            const dav_size_t s_chunk = (req.getAnswerSize() > 0)?(req.getAnswerSize()):DAVIX_BLOCK_SIZE;
            buffer.reserve(buffer.size()+ s_chunk);

            while ( (ret= req.readBlock( buffer, s_chunk, &tmp_err)) > 0){
                total += (dav_size_t) ret;
            }
            if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
                httpcodeToDavixError(req.getRequestCode(),davix_scope_io_buff(),"read error: ", &tmp_err);
                ret = -1;
            }
        }
    }

    DAVIX_DEBUG(" <- readFull on vector");
    checkDavixError(&tmp_err);
    return (ret>0)?total:-1;
}


dav_ssize_t HttpIO::pread(IOChainContext & iocontext, void *buf, dav_size_t count, dav_off_t offset){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;
    DAVIX_DEBUG(" -> getOps operation for %s with size %ld and offset %ld", iocontext._uri.getString().c_str(), count, offset);
    if(count ==0)
        return 0;

    HttpRequest req(iocontext._context, iocontext._uri, &tmp_err);
    if(tmp_err == NULL){
        RequestParams params(iocontext._reqparams);
        req.setParameters(params);
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
                    httpcodeToDavixError(req.getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                }
            }
        }
        req.endRequest(NULL);
    }
    DAVIX_DEBUG(" end getOps operation for %s <- ",iocontext._uri.getString().c_str());
    checkDavixError(&tmp_err);
    return ret;
}


dav_ssize_t HttpIO::readToFd(IOChainContext & iocontext, int fd, dav_size_t read_size){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;

    DAVIX_DEBUG(" -> readToFd for size %ld", read_size);
    GetRequest req (iocontext._context, iocontext._uri, &tmp_err);
    if(!tmp_err){
        RequestParams params(iocontext._reqparams);
        req.setParameters(iocontext._reqparams);
        ret = req.beginRequest(&tmp_err);
        if(!tmp_err){
            if(httpcodeIsValid(req.getRequestCode()) == false){
                httpcodeToDavixError(req.getRequestCode(),davix_scope_io_buff(),"read error: ", &tmp_err);
                ret = -1;
            }else{
                ret= req.readToFd(fd, read_size, &tmp_err);
            }
        }
    }


    DAVIX_DEBUG(" <- readToFd for size %ld", read_size);
    checkDavixError(&tmp_err);
    return ret;
}


// position independant write operation,
// similar to pwrite do not need open() before
dav_ssize_t HttpIO::writeFromFd(IOChainContext & iocontext, int fd, dav_size_t size){
    DavixError * tmp_err=NULL;
    dav_ssize_t ret = -1;

    DAVIX_DEBUG(" -> writeFromFd for size %ld", size);
    PutRequest req (iocontext._context,iocontext._uri, &tmp_err);
    if(!tmp_err){
        RequestParams params(iocontext._reqparams);
        req.setParameters(params);
        req.setRequestBody(fd,0, size);
        ret = req.executeRequest(&tmp_err);
        if(!tmp_err && httpcodeIsValid(req.getRequestCode()) == false){
            httpcodeToDavixError(req.getRequestCode(), davix_scope_io_buff(),
                                "read error: ", &tmp_err);
            ret = -1;
        }
    }


    DAVIX_DEBUG(" <- writeFromFd for size %ld", size);
    checkDavixError(&tmp_err);
    return ret;
}



/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///


struct IOBufferLocalFile{
    IOBufferLocalFile(int fd, const std::string & filepath): _fd(fd), _filepath(filepath){}
    virtual ~IOBufferLocalFile(){
        DAVIX_TRACE("Delete tmp file %s", _filepath.c_str());
        unlink(_filepath.c_str());
    }

    int _fd;
    std::string _filepath;
};

HttpIOBuffer::HttpIOBuffer() :
    HttpIOChain(),
    _file_size(0),
    _file_exist(false),
    _pos(false),
    _opened(false),
    _last_advise(AdviseAuto),
    _rwlock(),
    _read_pos(0),
    _read_endfile(false),
    _read_req(NULL)
{

}

HttpIOBuffer::~HttpIOBuffer(){
    delete _read_req;
}

IOBufferLocalFile* createLocalBuffer(){
    char buffer[1024];
    strcpy(buffer, "/tmp/.davix_tmp_file_XXXXXXXXXXXXXXXXXXXXXXXX");
    int fd;
    if( (fd = mkstemp(buffer)) < 0){
        DAVIX_TRACE("Error during temporary file creation for HTTPIO %s: %s", buffer, strerror(errno));
        return NULL;
    }
    return new IOBufferLocalFile(fd, buffer);
}

bool HttpIOBuffer::open(IOChainContext & iocontext, int flags){
    bool res = false;
    if(_opened)
        return true;

    struct StatInfo infos;

    try{
        _start->statInfo(iocontext, infos);

        if( (flags & O_EXCL) && ( flags & O_CREAT)){
            throw DavixException(davix_scope_io_buff(),
                                   StatusCode::FileExist, "file exist and O_EXCL flag usedin open");
        }else{
            _file_size = infos.size;
            _file_exist = true;
            _opened = true;
        }
    }catch(DavixException & e){
        if(e.code() == StatusCode::FileNotFound
                &&  (flags & O_CREAT)
                && ((flags & O_RDWR) || (flags  & O_WRONLY))){
            _file_size = 0;
            _file_exist = false;
            _opened = true;
            _local.reset(createLocalBuffer());
        }else{
            throw e;
        }
    }

    DAVIX_TRACE("File open %s, size: %ld", iocontext._uri.getString().c_str(), _file_size);
    return res;
}

dav_ssize_t HttpIOBuffer::read(IOChainContext & iocontext, void *buf, dav_size_t count){
    boost::recursive_mutex::scoped_lock l(_rwlock);
    DavixError* tmp_err = NULL;
    dav_ssize_t ret =-1;

    if(_pos ==0) // reset read ahead offset to default if try to read a full file
        resetIO(iocontext);
    if(_pos == _read_pos && isAdviseFullRead()){
        // try read ahead strategie
        ret = readInternal(iocontext, buf, count);
    }else{ // fallback on partial read
        ret = _start->pread(iocontext, buf, count, _pos);
    }
    if(ret > 0)
        _pos += ret;

    checkDavixError(&tmp_err);
    return ret;
}


dav_ssize_t HttpIOBuffer::readInternal(IOChainContext & iocontext, void *buffer, dav_size_t size_read){
    dav_ssize_t ret = -1;
    DavixError * tmp_err=NULL;


    if(_read_endfile)
        return 0;

    if( _read_req == NULL
            && (_read_req = new HttpRequest(iocontext._context, iocontext._uri, &tmp_err)) != NULL
            && tmp_err == NULL ){
        RequestParams params(iocontext._reqparams);
        _read_req->setParameters(params);
        if(_read_req->beginRequest(&tmp_err) ==0
            && (_read_req->getRequestCode() != 200)){
                httpcodeToDavixError(_read_req->getRequestCode(),davix_scope_http_request(),", while  readding", &tmp_err);
                delete _read_req;
                _read_req = NULL;

        }
        if(tmp_err){
            delete _read_req;
            _read_req = NULL;
            ret = -1;
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

    checkDavixError(&tmp_err);
    return ret;
}



void HttpIOBuffer::prefetchInfo(IOChainContext & iocontext, off_t offset, dav_size_t size_read, advise_t adv){
    _last_advise = adv;
}


void HttpIOBuffer::resetIO(IOChainContext & iocontext){
    boost::recursive_mutex::scoped_lock l(_rwlock);

    if(_read_req){
        delete _read_req;
        _read_req = NULL;
    }
    _read_pos =0;
    commitLocal(iocontext);
}


void HttpIOBuffer::commitLocal(IOChainContext & iocontext){
    boost::recursive_mutex::scoped_lock l(_rwlock);
    if(_local.get()){
        struct stat st;
        memset(&st,0, sizeof(struct stat));
        fstat(_local->_fd, &st);
        DAVIX_TRACE("Commit local file modifications, %d bytes", st.st_size);
        _start->writeFromFd(iocontext, _local->_fd, st.st_size);
        _local.reset();
    }
}


dav_off_t HttpIOBuffer::lseek(IOChainContext & iocontext, dav_off_t offset, int flags){
    boost::recursive_mutex::scoped_lock l(_rwlock);
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

dav_ssize_t HttpIOBuffer::write(IOChainContext & iocontext, const void *buf, dav_size_t count){
    boost::recursive_mutex::scoped_lock l(_rwlock);
    dav_ssize_t ret =-1;
    dav_size_t write_len = count;


    if(_local.get() == NULL || !_opened){
        throw DavixException(davix_scope_io_buff(), StatusCode::SystemError, "Impossible to write, I/O Error");
    }

    do{
        ret = pwrite(_local->_fd, buf, static_cast<size_t>(count), _pos);

        if (ret == -1 && errno == EINTR) {
            continue;
        } else if (ret < 0) {
            throw DavixException(davix_scope_io_buff(),
                                   StatusCode::SystemError, std::string("Impossible to write to fd").append(strerror(errno)));
        } else {
            _pos += ret;
            write_len -= ret;
        }
    }while(write_len >0);

    return (count - write_len);
}




} // namespace Davix
