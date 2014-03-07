/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
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
