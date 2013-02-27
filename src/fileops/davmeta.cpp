#include "davmeta.hpp"

namespace Davix{

static const std::string propfind_request_replicas("<D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\">"
                                                   "<D:prop><L:replicas/></D:prop>"
                                                   "</D:propfind>");


// get all reps from webdav queries
ssize_t Webdav_getAllReplicas(const RequestParams & params, ReplicaVec & vec, DavixError** err){
    ssize_t ret =-1;


    return ret;
}



} // Davix
