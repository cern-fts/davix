#include <config.h>

#include <davix_types.h>
#include <httprequest.hpp>
#include <posix/davposix.hpp>
#include <logger/davix_logger_internal.h>
#include <http_util/http_util.hpp>
#include "iobuffmap.hpp"

#include <sstream>
#include <string>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>



const std::string req_header_byte_range("Range");
const std::string ans_header_content_range("Content-Type");
const std::string ans_header_content_length("Content-Length");
const std::string ans_header_multi_part_value("multipart");
const std::string ans_header_boundary_field("boundary=");
const std::string offset_value("bytes=");

namespace Davix {

ssize_t read_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err);
ssize_t read_truncated_segment_request(HttpRequest* req, void* buffer, size_t size_read,  off_t off_set, DavixError**err);



static const dav_size_t get_answer_file_size(const HttpRequest&  req, DavixError**err){
    std::string str_file_size;
    dav_size_t size=-1;
    if( req.getAnswerHeader(ans_header_content_length, str_file_size)){
        size = (dav_size_t) strtoul(str_file_size.c_str(), NULL, 10);
    }
    if( size == -1 || size == ULONG_MAX){
        DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                               std::string("Bad server answer: ") + ans_header_content_length + "Invalid ");
        return -1;
    }
}

void setup_offset_request(HttpRequest* req, const off_t *start_len, const size_t *size_read, const size_t number_ops){
    std::ostringstream buffer;
    buffer << offset_value;

    for(size_t i = 0; i<number_ops; ++i){
        if( i > 0)
            buffer << ",";

       if(size_read > 0)
           buffer << start_len[i] << "-"<< (start_len[i]+size_read[i]);
       else
            buffer << start_len [i]<< "-";
     }
     req->addHeaderField(req_header_byte_range, buffer.str());

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
        setup_offset_request(req.get(), &offset, &count,1);
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

void configure_iovec_range_header(HttpRequest* req, const DavIOVecInput * input_vec, const dav_size_t count_vec){
    off_t offset_tab[count_vec];
    size_t size_tab[count_vec];

    for(dav_size_t i =0; i < count_vec; ++i){
        offset_tab[i] = input_vec[i].diov_offset;
        size_tab[i] = input_vec[i].diov_size;
    }
    setup_offset_request(req, offset_tab, size_tab, count_vec);
}


int check_multi_part_content_type(const HttpRequest& req, std::string & boudary, DavixError** err){

    size_t pos_bound;
    std::string buffer;
    int ret;


    if( req.getAnswerHeader(ans_header_content_range, buffer) == true // has content type
           && ( strncmp(buffer.c_str(), ans_header_multi_part_value.c_str(), ans_header_multi_part_value.size()) == 0) // has multipart
           && ( ( pos_bound= buffer.find(ans_header_boundary_field)) != std::string::npos)){
          char* p, *origin_p;
          origin_p = p = (char*) buffer.c_str() + pos_bound;
          while( isValidHeaderChar(p))
              ++p;
          if( p == origin_p ){
              DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse, "Invalid boundary field in multipart content type");
              ret = -1;
          }
          boudary.assign(buffer.c_str(), p - buffer.c_str());
          ret =0;
    }else{
        DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse, "Bad server response, not a multi-part response");
        ret = -1;
    }
    DAVIX_TRACE("Multi part content type : %s", boudary.c_str());
    return ret;

}

dav_ssize_t parse_multi_part_find_next_part(HttpRequest* req, dav_off_t* off, dav_size_t* size, DavixError** err){

}



dav_ssize_t parse_multipart_request(HttpRequest* req, const DavIOVecInput * input_vec,
                                    DavIOVecOuput * output_vec,
                                    const dav_size_t count_vec, DavixError** err){
    std::string boundary;
    dav_size_t file_size, total_read_bytes=0, current_read_bytes, n_vec=0;

    if(( file_size = get_answer_file_size(*req, err))== -1
          || check_multi_part_content_type(*req, boundary, err)  != 0 )
        return -1;

   /* while( total_read_bytes < file_size && n_vec < count_vec){
        bool new_part = false
        while(!new_part){
            if( ( current_read_bytes= req->readBlock(buffer, DAVIX_READ_BLOCK_SIZE, err)) >= 0)

    }*/




}

dav_ssize_t HttpIO::readPartialBufferVec(const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      const dav_size_t count_vec, DavixError** err){
    DavixError * tmp_err=NULL;
    ssize_t ret = -1;
    if(count_vec ==0)
        return 0;
    DAVIX_DEBUG(" -> getPartialVec operation for %d vectors", count_vec);
    std::auto_ptr<HttpRequest> req (_c.createRequest(_uri, &tmp_err));
    if(req.get() != NULL){
        req->setParameters(_params);
        configure_iovec_range_header(req.get(), input_vec, count_vec);
        if( req->beginRequest(&tmp_err) >0){
            if(req->getRequestCode() == 206 ){  // multipart req
                ret = parse_multipart_request(req.get() , input_vec,
                                              output_vec, count_vec, &tmp_err);
            }else if( req->getRequestCode() == 200){ // full request content , no multi part
                // TODO
                return -1;
            }else{ // error
                httpcodeToDavixCode(req->getRequestCode(),davix_scope_http_request(),", ", &tmp_err);
            }
        }
    }


    DAVIX_DEBUG(" <- getPartialVec operation for %d vectors", count_vec);
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

dav_ssize_t HttpIOBuffer::preadVec(const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      dav_size_t count_vec, DavixError** err){
    davix_return_val_if_fail( input_vec != NULL && output_vec != NULL,-1);
    dav_ssize_t res = -1;
    if(count_vec ==1){ // one offset read request, no need of multi part
        res= (dav_ssize_t) pread(input_vec->diov_buffer, (size_t) input_vec->diov_size, (off_t) input_vec->diov_offset, err);
        output_vec->diov_buffer = input_vec->diov_buffer;
        output_vec->diov_size= res;

    }else{  // setup multi part transfer
        res = readPartialBufferVec(input_vec, output_vec, count_vec, err);
    }
    return res;
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
