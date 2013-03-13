#include <config.h>
#include <logger/davix_logger_internal.h>
#include <fileops/fileutils.hpp>
#include <fileops/davmeta.hpp>


#include "davix_stat.hpp"




namespace Davix{



int DavPosix::stat(const RequestParams * params, const std::string & url, struct stat* st, DavixError** err){
    DAVIX_DEBUG(" -> davix_stat");
    DavixError* tmp_err=NULL;

    int ret = Meta::posixStat(*context, Uri(url), params, st, NULL, &tmp_err);

    DAVIX_DEBUG(" davix_stat <-");
    if(tmp_err)
        DavixError::propagatePrefixedError(err, tmp_err, "stat ops : ");
    return ret;

}




}


DAVIX_C_DECL_BEGIN

int davix_posix_stat(davix_sess_t sess, davix_params_t _params, const char* url, struct stat * st, davix_error_t* err){
    davix_return_val_if_fail(sess != NULL,-1);

    Davix::DavPosix p((Davix::Context*)(sess));
    Davix::RequestParams * params = (Davix::RequestParams*) (_params);

    return p.stat(params,url, st, (Davix::DavixError**) err);

}

DAVIX_C_DECL_END
