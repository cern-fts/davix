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
    ne_session * ne_sess_ptr=NULL;
    context->_intern->getSessionFactory()->createNeonSession(this->getUri(),&(ne_sess_ptr));
    configure_sess(ne_sess_ptr);
    res->_sess= ne_sess_ptr;

    // create and configure request
    ne_request* req=NULL;

    return res;
}

} // namespace Davix
