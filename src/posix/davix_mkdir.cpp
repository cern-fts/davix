#include <ostream>
#include <sstream>

#include <posix/davposix.hpp>
#include <status/davixstatusrequest.hpp>
#include <fileops/fileutils.hpp>


namespace Davix{

int DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right, DavixError** err){
    DAVIX_DEBUG(" -> davix_mkdir");
    int ret=-1;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);


    std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(context->createRequest(url, &tmp_err)));
    if(req.get() != NULL){

        req->setParameters(params);
        req->setRequestMethod("MKCOL");

        if( (ret = req->executeRequest(&tmp_err)) == 0){
            ret = davixRequestToFileStatus(req.get(), davix_scope_mkdir_str(), &tmp_err);
        }

        DAVIX_DEBUG(" davix_mkdir <-");
    }
    if(tmp_err)
        DavixError::propagateError(err, tmp_err);
    return ret;
}


}




DAVIX_C_DECL_BEGIN

int davix_posix_mkdir(davix_sess_t sess, davix_params_t _params, const char* url,  mode_t right, davix_error_t* err){
    g_return_val_if_fail(sess != NULL && url != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.mkdir(params,url, right, (Davix::DavixError**) err);
}

DAVIX_C_DECL_END


