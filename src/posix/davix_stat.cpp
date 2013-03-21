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
