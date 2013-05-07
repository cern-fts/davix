#include "davops.hpp"
#include <neon/neonsession.hpp>
#include <ne_basic.h>


namespace Davix{


static int DavOpsDelete(Context & c, const RequestParams & params, const Uri & uri, DavixError** err){

    int ret =-1;
    DavixError* tmp_err=NULL;
    if(uriCheckError(uri, &tmp_err)){
        NEONSession s(c,uri, params, &tmp_err);
        if(!tmp_err){
            if( (ret = ne_delete(s.get_ne_sess(), uri.getPathAndQuery().c_str())) != NE_OK){
                neon_simple_req_code_to_davix_code(ret, s.get_ne_sess(), davix_scope_davOps_str(), &tmp_err);
                ret = -1;
            }
        }
    }

    if(tmp_err)
        DavixError::propagatePrefixedError(err,tmp_err, "delete ops : ");
    return ret;
}

WebdavQuery::WebdavQuery(Context &c) :
    _c(c)
{

}



int WebdavQuery::davDelete(const RequestParams * params, const Uri &uri, DavixError **err){
    RequestParams _params(params);
    return DavOpsDelete(_c, _params, uri, err);
}



}
