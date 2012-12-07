#include "davix_mkdir.hpp"

#include <ostream>
#include <sstream>

#include <global_def.hpp>
#include <davixcontext.hpp>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>
#include <fileops/davops.hpp>


namespace Davix{


int davix_remove_posix(DavPosix & p, Context* c, const RequestParams & params, const std::string & url, bool directory, DavixError** err){
    DavixError* tmp_err = NULL;
    int ret = -1;
    Uri uri(url);
    WebdavQuery query(*c);

    if(params.getProtocol() == DAVIX_PROTOCOL_HTTP){ // pure protocol http : ignore posix semantic, execute a simple delete
        ret = query.davDelete(params, uri, &tmp_err);
    }else{ // full posix semantic support
        struct stat st;
        ret = p.stat(&params, url, &st, &tmp_err);
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
                    DavixError::setupError(&tmp_err, davix_scope_davOps_str(), StatusCode::isNotADirectory, ss.str());
                }
            }
        }
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;

}


int DavPosix::unlink(const RequestParams * _params, const std::string &url, DavixError** err){
    davix_log_debug(" -> davix_unlink");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);

    ret = davix_remove_posix(*this, context, _params, url, false, &tmp_err);
    davix_log_debug(" davix_unlink <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::unlink ");
    return ret;
}


int DavPosix::rmdir(const RequestParams * _params, const std::string &url, DavixError** err){
    davix_log_debug(" -> davix_rmdir");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);

    ret = davix_remove_posix(*this, context, _params, url, true, &tmp_err);
    davix_log_debug(" davix_rmdir <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "DavPosix::rmdir ");
    return ret;
}


}




DAVIX_C_DECL_BEGIN

int davix_posix_unlink(davix_sess_t sess, davix_params_t _params, const char* url,   davix_error_t* err){
    g_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.unlink(params, url,  (Davix::DavixError**) err);
}

int davix_posix_rmdir(davix_sess_t sess, davix_params_t _params, const char* url,   davix_error_t* err){
    g_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.rmdir(params, url,  (Davix::DavixError**) err);
}

DAVIX_C_DECL_END


