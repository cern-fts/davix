#include <ostream>
#include <sstream>
#include <memory>
#include <logger/davix_logger_internal.h>
#include <posix/davposix.hpp>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>


namespace Davix{

int DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right, DavixError** err){
    DAVIX_DEBUG(" -> davix_mkdir");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);
    HttpRequest req(*context, url, &tmp_err);

    if(tmp_err == NULL){

        req.setParameters(params);
        req.setRequestMethod("MKCOL");

        if( (ret = req.executeRequest(&tmp_err)) == 0){
            ret = davixRequestToFileStatus(&req, davix_scope_mkdir_str(), &tmp_err);
        }

        DAVIX_DEBUG(" davix_mkdir <-");
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


}



