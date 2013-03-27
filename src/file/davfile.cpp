#include "davfile.hpp"
#include <fileops/davmeta.hpp>
#include <fileops/iobuffmap.hpp>
#include <fileops/davops.hpp>

namespace Davix{

struct DavFileInternal{
    DavFileInternal(Context & c, const Uri & u) :
        _c(c), _u(u) {}

    Context & _c;
    Uri _u;
};


DavFile::DavFile(Context &c, const Uri &u) :
    d_ptr(new DavFileInternal(c,u))
{

}

DavFile::~DavFile(){
    delete d_ptr;
}


dav_ssize_t DavFile::getAllReplicas(const RequestParams* params, ReplicaVec &vec, DavixError **err){
    return (dav_ssize_t) Meta::getAllReplicas(d_ptr->_c, d_ptr->_u, *params, vec, err);
}


dav_ssize_t DavFile::readPartialBufferVec(const RequestParams *params, const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      const dav_size_t count_vec, DavixError** err){
    HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
    return (dav_ssize_t) io.readPartialBufferVec(input_vec, output_vec, count_vec, err);
}


dav_ssize_t DavFile::readPartial(const RequestParams *params, void* buff, dav_size_t count, dav_off_t offset, DavixError** err){
    HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
    return (dav_ssize_t) io.readPartialBuffer(buff, count, offset, err);
}

int DavFile::deletion(const RequestParams *params, DavixError **err){
    return Meta::deleteResource(d_ptr->_c, d_ptr->_u, params, err);
}


} //Davix
