#include "neongenericrequest.h"
#include <abstractsessionfactory.hpp>
#include <contextinternal.h>


namespace Davix {

int ne_ssl_verify_fn_accept_all(	void *userdata,
    int failures,
    const ne_ssl_certificate *cert){
    return 0;
}

NeonGenericRequest::NeonGenericRequest(Context* context, const Uri & uri): NGQHttpRequest(context, uri)
{
}

void NeonGenericRequest::addHeaderField(const std::string &field, const std::string &value){
    _headers.push_back(HeaderField(field, value));
}

void NeonGenericRequest::configure_sess(ne_session* sess){
    if( context->getSSLCACheck()== false){ // configure ssl check
        davix_log_debug("NeonGenericRequest : disable ssl verification");
        ne_ssl_set_verify(sess,ne_ssl_verify_fn_accept_all , NULL);
    }else{
         ne_ssl_trust_default_ca(sess);
    }

    const int ops_timeout= (int) context->getOperationTimeout()->tv_sec;
    if( ops_timeout> 0){
        davix_log_debug("NeonGenericRequest : define operation timeout to %d", ops_timeout);
        ne_set_read_timeout(sess, (int) ops_timeout  );
    }
    const int conn_timeout = (int) context->getOperationTimeout()->tv_sec;
    if(conn_timeout > 0){
        davix_log_debug("NeonGenericRequest : define connexion timeout to %d", conn_timeout);
#ifndef _NEON_VERSION_0_25
        ne_set_connect_timeout(sess, (int) conn_timeout );
#endif
    }
}


NeonRequestStatus* NeonGenericRequest::executeRequest(){
    // create and configure session
    NeonRequestStatus* res = new NeonRequestStatus(context, this);
    res->code = StatusCode::UnknowError;

    ne_session * ne_sess_ptr=NULL;
    davix_log_debug("NeonGenericRequest : construct Session for  %s %s", ops.c_str(), _uri.getString().c_str());
    context->_intern->getSessionFactory()->createNeonSession(this->getUri(),&(ne_sess_ptr));
    configure_sess(ne_sess_ptr);

    if(ne_sess_ptr != NULL){
        res->_sess= ne_sess_ptr;

        // create and configure request
        ne_request* req=NULL;
        davix_log_debug("NeonGenericRequest : construct Request for  %s %s", ops.c_str(), _uri.getString().c_str());
        req = ne_request_create(ne_sess_ptr, ops.c_str(), _uri.getPath().c_str());
        if(req != NULL){
            for(size_t i=0; i< _headers.size(); ++i){
                ne_add_request_header(req, _headers[i].first.c_str(),  _headers[i].second.c_str());
            }
            if(_body_size > 0)
                ne_set_request_body_buffer(req, (const char*) _body, _body_size);
            res->_req = req;
        }else{
            res->code = StatusCode::UriParsingError;
            res->err_msg = std::string("Unable to construct a Valid request for  ") + ops + std::string(" ") + _uri.getString();
        }

    }else{
        res->code = StatusCode::SessionCreationError;
        res->err_msg = std::string("Unable to construct a Valid session for  ") + ops + std::string(" ") + _uri.getString();
    }
    return res;
}

} // namespace Davix
