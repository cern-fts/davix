#include "davfile.hpp"
#include <fileops/davmeta.hpp>
#include <fileops/iobuffmap.hpp>

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
    return (dav_ssize_t) webdavGetAllReplicas(d_ptr->_c, d_ptr->_u, *params, vec, err);
}



///
/// @brief return all replicas associated to this file
///
/// Replicas are found using a corresponding meta-link file or Webdav extensions if supported
///
/// @param params: Davix Request parameters
/// @param vec : Replica vector
/// @param err : DavixError error report
/// @return  the number of replicas if found, -1 if error.
dav_ssize_t DavFile::readPartialBufferVec(const RequestParams *params, const DavIOVecInput * input_vec,
                      DavIOVecOuput * output_vec,
                      const dav_size_t count_vec, DavixError** err){
    HttpIOBuffer io(d_ptr->_c, d_ptr->_u, params);
    return io.readPartialBufferVec(input_vec, output_vec, count_vec, err);
}

} //Davix
