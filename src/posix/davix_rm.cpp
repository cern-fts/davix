#include <config.h>
#include <ostream>
#include <sstream>

#include <logger/davix_logger_internal.h>
#include <posix/davposix.hpp>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/davops.hpp>


namespace Davix{


int davix_remove_posix(DavPosix & p, Context* c, const RequestParams * params, const std::string & url, bool directory, DavixError** err){
    DavixError* tmp_err = NULL;
    int ret = -1;
    Uri uri(url);
    WebdavQuery query(*c);

    if(params && params->getProtocol() == DAVIX_PROTOCOL_HTTP){ // pure protocol http : ignore posix semantic, execute a simple delete
        ret = query.davDelete(params, uri, &tmp_err);
    }else{ // full posix semantic support
        struct stat st;
        ret = p.stat(params, url, &st, &tmp_err);
        if( ret ==0){
            if( S_ISDIR(st.st_mode)){ // directory : impossible to delete if not empty
                if(directory == true){
                    // ignore non empty dir check for now
                    ret = query.davDelete(params, uri, &tmp_err);
                }else{
                    ret = -1;
                    std::ostringstream ss;
                    ss << " " << url << " is not a directory, impossible to unlink"<< std::endl;
                    DavixError::setupError(&tmp_err, davix_scope_davOps_str(), StatusCode::IsADirectory, ss.str());
                }
            }else{ // file, rock & roll
                if(directory == false){
                    ret = query.davDelete(params, uri, &tmp_err);
                }else{
                    ret = -1;
                    std::ostringstream ss;
                    ss << " " << url << " is not a directory, impossible to rmdir"<< std::endl;
                    DavixError::setupError(&tmp_err, davix_scope_davOps_str(), StatusCode::IsNotADirectory, ss.str());
                }
            }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;

}


int DavPosix::unlink(const RequestParams * params, const std::string &url, DavixError** err){
    DAVIX_DEBUG(" -> davix_unlink");
    int ret=-1;
    DavixError* tmp_err=NULL;

    ret = davix_remove_posix(*this, context, params, url, false, &tmp_err);
    DAVIX_DEBUG(" davix_unlink <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::unlink ");
    return ret;
}


int DavPosix::rmdir(const RequestParams * params, const std::string &url, DavixError** err){
    DAVIX_DEBUG(" -> davix_rmdir");
    int ret=-1;
    DavixError* tmp_err=NULL;

    ret = davix_remove_posix(*this, context, params, url, true, &tmp_err);
    DAVIX_DEBUG(" davix_rmdir <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::rmdir ");
    return ret;
}


}







