#include <davix_internal.hpp>
#include <file/davfile.hpp>
#include <fileops/davmeta.hpp>
#include <fileops/iobuffmap.hpp>
#include <fileops/davops.hpp>

namespace Davix{

struct DavFile::DavFileInternal{
    DavFileInternal(Context & c, const Uri & u) :
        _c(c), _u(u) {}

    DavFileInternal(const DavFileInternal & orig) : _c(orig._c), _u(orig._u) {}

    Context & _c;
    Uri _u;
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
    RequestParams params(_params);
    TRY_DAVIX{
        Meta::getReplicas(d_ptr->_c,d_ptr->_u, params, res);
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
        HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
        return (dav_ssize_t) io.readPartialBufferVec(input_vec, output_vec, count_vec, err);
    }CATCH_DAVIX(err)
    return -1;
}


dav_ssize_t DavFile::readPartial(const RequestParams *params, void* buff, dav_size_t count, dav_off_t offset, DavixError** err){
    TRY_DAVIX{
        HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
        return (dav_ssize_t) io.readPartialBuffer(buff, count, offset, err);
    }CATCH_DAVIX(err)
    return -1;
}

int DavFile::deletion(const RequestParams *params, DavixError **err){
    TRY_DAVIX{
        return Meta::deleteResource(d_ptr->_c, d_ptr->_u, params, err);
    }CATCH_DAVIX(err)
    return -1;
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
        HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
        return io.readToFd(fd, size_read, err);
    }CATCH_DAVIX(err)
    return -1;
}



dav_ssize_t DavFile::getFull(const RequestParams* params,
                        std::vector<char> & buffer,
                        DavixError** err){
    TRY_DAVIX{
        HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
        return io.readFull(buffer, err);
    }CATCH_DAVIX(err)
    return -1;
}


int DavFile::putFromFd(const RequestParams* params,
              int fd,
              dav_size_t size,
              DavixError** err){
    TRY_DAVIX{
        HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
        const dav_ssize_t s = io.writeFullFromFd(fd, size, err);
        return (s >= 0)?0:-1;
    }CATCH_DAVIX(err)
    return -1;
}


int DavFile::makeCollection(const RequestParams *params, DavixError **err){
    RequestParams _params(params);
    TRY_DAVIX{
        return Meta::makeCollection(d_ptr->_c, d_ptr->_u, _params, err);
    }CATCH_DAVIX(err)
    return -1;
}

int DavFile::stat(const RequestParams* params, struct stat * st, DavixError** err){
    TRY_DAVIX{
        return (int) Meta::posixStat(d_ptr->_c, d_ptr->_u, params, st, err);
    }CATCH_DAVIX(err)
    return -1;
}

void DavFile::prefetchInfo(off_t offset, dav_size_t size_read, advise_t adv){
    // TODO
}


} //Davix
