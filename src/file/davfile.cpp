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





#include <davix_internal.hpp>
#include <file/davfile.hpp>
#include <fileops/davmeta.hpp>
#include <fileops/iobuffmap.hpp>
#include <fileops/httpiovec.hpp>

namespace Davix{

struct DavFile::DavFileInternal{
    DavFileInternal(Context & c, const Uri & u) :
        _c(c), _u(u) {}

    DavFileInternal(const DavFileInternal & orig) : _c(orig._c), _u(orig._u) {}

    Context & _c;
    Uri _u;
    HttpIOChain & getIOChain(HttpIOChain & c, const RequestParams *params){
        c.add(new HttpMetaOps())->add(new HttpIO())->add(new HttpIOVecOps());
        c.configure(_c, _u, params);
        return c;
    }
};


DavFile::DavFile(Context &c, const Uri &u) :
    d_ptr(new DavFileInternal(c,u))
{

}

DavFile::DavFile(const DavFile & orig): d_ptr(new DavFileInternal(*orig.d_ptr)){

}

DavFile::~DavFile(){
    delete d_ptr;
}


const Uri &  DavFile::getUri() const{
    return d_ptr->_u;
}


std::vector<DavFile> DavFile::getReplicas(const RequestParams *_params, DavixError **err){
    std::vector<DavFile> res;
    TRY_DAVIX{
        HttpIOChain chain;
        return d_ptr->getIOChain(chain,_params).getReplicas(res);
    }CATCH_DAVIX(err)
    return res;
}


dav_ssize_t DavFile::getAllReplicas(const RequestParams* params, ReplicaVec & v, DavixError **err){
    Davix::DavixError::setupError(err, davix_scope_http_request(), StatusCode::OperationNonSupported, " GetAllReplicas Function not supported, please use GetReplicas()");
    return -1;
}

dav_ssize_t DavFile::readPartialBufferVec(const RequestParams *params, const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      const dav_size_t count_vec, DavixError** err){
    TRY_DAVIX{
        HttpIOChain chain;
        return d_ptr->getIOChain(chain,params).preadVec(input_vec, output_vec, count_vec);
    }CATCH_DAVIX(err)
    return -1;
}


dav_ssize_t DavFile::readPartial(const RequestParams *params, void* buff, dav_size_t count, dav_off_t offset, DavixError** err){
    TRY_DAVIX{
        HttpIOChain chain;
        return d_ptr->getIOChain(chain, params).pread(buff, count, offset);
    }CATCH_DAVIX(err)
    return -1;
}

int DavFile::deletion(const RequestParams *params, DavixError **err) throw(){
    TRY_DAVIX{
        deletion(params);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}

void DavFile::deletion(const RequestParams *params){
    HttpIOChain chain;
    d_ptr->getIOChain(chain, params).deleteResource();
}

dav_ssize_t DavFile::getToFd(const RequestParams* params,
                        int fd,
                        DavixError** err){
    TRY_DAVIX{
        return DavFile::getToFd(params, fd, 0, err);
    }CATCH_DAVIX(err)
    return -1;
}

dav_ssize_t DavFile::getToFd(const RequestParams* params,
                        int fd,
                        dav_size_t size_read,
                        DavixError** err){
    TRY_DAVIX{
        HttpIOChain chain;
        return d_ptr->getIOChain(chain, params).readToFd(fd, size_read);
    }CATCH_DAVIX(err)
    return -1;
}



dav_ssize_t DavFile::getFull(const RequestParams* params,
                        std::vector<char> & buffer,
                        DavixError** err){
    TRY_DAVIX{
        HttpIOChain chain;
        return d_ptr->getIOChain(chain, params).readFull(buffer);
    }CATCH_DAVIX(err)
    return -1;
}


int DavFile::putFromFd(const RequestParams* params,
              int fd,
              dav_size_t size,
              DavixError** err){
    TRY_DAVIX{
        HttpIOChain chain;
        d_ptr->getIOChain(chain, params).writeFromFd(fd, size);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}


int DavFile::makeCollection(const RequestParams *params, DavixError **err) throw(){
    TRY_DAVIX{
        makeCollection(params);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}


void DavFile::makeCollection(const RequestParams *params){
    RequestParams _params(params);
    HttpIOChain chain;
    d_ptr->getIOChain(chain,params).makeCollection();
}

int DavFile::stat(const RequestParams* params, struct stat * st, DavixError** err){
    TRY_DAVIX{
        if(st == NULL)
            throw DavixException(davix_scope_meta(), StatusCode::InvalidArgument, "Argument stat NULL");

        StatInfo info;
        HttpIOChain chain;
        d_ptr->getIOChain(chain,params).statInfo(info);
        info.toPosixStat(*st);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}

int DavFile::checksum(const RequestParams *params, std::string & checksm, const std::string & chk_algo, DavixError **err) throw(){
    TRY_DAVIX{
        HttpIOChain chain;
        d_ptr->getIOChain(chain,params).checksum(checksm, chk_algo);
        return 0;
    }CATCH_DAVIX(err)
    return -1;
}

void DavFile::prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv){
    // TODO
}


} //Davix
