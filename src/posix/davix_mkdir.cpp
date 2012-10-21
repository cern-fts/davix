#include "davix_mkdir.hpp"

#include <ostream>
#include <sstream>

#include <global_def.hpp>
#include <davixcontext.hpp>
#include <contextinternal.h>
#include <xmlpp/webdavpropparser.hpp>


namespace Davix{

int DavPosix::mkdir(const RequestParams * _params, const std::string &url, mode_t right, DavixError** err){
    davix_log_debug(" -> davix_mkdir");
    int ret;
    DavixError* tmp_err=NULL;
    RequestParams params(_params);


    WebdavPropParser parser;
    std::auto_ptr<HttpRequest> req( static_cast<HttpRequest*>(context->_intern->getSessionFactory()->create_request(url, &tmp_err)));
    if(req.get() != NULL){

        req->set_parameters(params);
        req->setRequestMethod("MKCOL");

        if( (ret = req->execute_sync(&tmp_err)) == 0){

            if(httpcodeIsValid(req->getRequestCode())){
               ret =0;
            }else{
                httpcodeToDavixCode(req->getRequestCode(), davix_scope_mkdir_str(),"", &tmp_err);
                ret = -1;
            }
        }

        davix_log_debug(" davix_mkdir <-");
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


