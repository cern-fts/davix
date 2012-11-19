#include "davops.hpp"
#include <neon/neonsession.hpp>
#include <ne_basic.h>


namespace Davix{

int DavOpsDelete(Context & c, const RequestParams & params, const Uri & uri, DavixError** err){

    int ret =-1;
    DavixError* tmp_err=NULL;
    if(UricheckError(uri, &tmp_err)){
        NEONSession s(c,uri, params, &tmp_err);
        if(!tmp_err){
            int ret;
            if( (ret = ne_delete(s.get_ne_sess(), uri.getPathAndQuery().c_str())) != NE_OK){
                neon_to_davix_code(ret, s.get_ne_sess(), davix_scope_davOps_str(), &tmp_err);
            }
        }
    }

    if(tmp_err)
        DavixError::propagatePrefixedError(err,tmp_err, "Davix Delete Operation : ");
    return ret;
}


}
