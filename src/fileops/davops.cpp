#include "davops.hpp"
#include <neon/neonsession.hpp>

namespace Davix{

int DavOpsDelete(Context & c, const RequestParams & params, const Uri & uri, DavixError** err){

    int ret =-1;
    DavixError* tmp_err=NULL;
    if(UricheckError(uri, &tmp_err)){
        NEONSession s(c,uri, params, &tmp_err);
        if(!tmp_err){

        }
    }

    if(tmp_err)
        DavixError::propagateError(err,tmp_err);
    return ret;
}


}
