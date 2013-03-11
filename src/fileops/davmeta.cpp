#include "davmeta.hpp"
#include <request/httprequest.hpp>

namespace Davix{

static const std::string propfind_request_replicas("<D:propfind xmlns:D=\"DAV:\" xmlns:L=\"LCGDM:\">"
                                                   "<D:prop><L:replicas/></D:prop>"
                                                   "</D:propfind>");


// get all reps from webdav queries
ssize_t webdavGetAllReplicas(Context & c, const Uri & uri,
                              const RequestParams & params, ReplicaVec & vec, DavixError** err){
    ssize_t ret =-1;
    DavixError* tmp_err=NULL;
    HttpRequest req(c, uri, &tmp_err);
    if(tmp_err == NULL){
        req.setParameters(params);
        req.setRequestMethod("PROPFIND");
        req.setRequestBodyString(propfind_request_replicas);
        if( req.executeRequest(&tmp_err) == 0){

        }
    }

    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}



} // Davix
