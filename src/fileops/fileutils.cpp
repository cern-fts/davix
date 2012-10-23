#include "fileutils.hpp"

namespace Davix {


int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err){
    const int reqcode = req->getRequestCode();
    int ret = 0;
    if(reqcode != 200 && reqcode != 207){
        DavixError* tmp_err=NULL;
        httpcodeToDavixCode(reqcode, scope, "",&tmp_err);
        if(tmp_err && tmp_err->getStatus() != StatusCode::OK){
            DavixError::propagateError(err, tmp_err);
            ret = -1;
        }else{
            DavixError::clearError(&tmp_err);
        }
    }
    return ret;
}

} // namespace Davix
