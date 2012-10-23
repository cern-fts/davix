#include <davixrequestparams.hpp>
#include <libs/time_utils.h>


namespace Davix {

RequestParams::RequestParams()
{
    _init();
}

RequestParams::RequestParams(const RequestParams& params){
    call = params.call;
    userdata = params.userdata;
    ssl_check = params.ssl_check;
    timespec_copy(&(connexion_timeout), &(params.connexion_timeout));
    timespec_copy(&(ops_timeout), &(params.ops_timeout));
    _redirection = params._redirection;
}

RequestParams::~RequestParams(){

}

RequestParams::RequestParams(const RequestParams* params){
    if(params){
        *this = *params;
    }else{
        _init();
    }
}




void RequestParams::_init(){
    timespec_clear(&connexion_timeout);
    timespec_clear(&ops_timeout);
    connexion_timeout.tv_sec = DAVIX_DEFAULT_CONN_TIMEOUT;
    ops_timeout.tv_sec = DAVIX_DEFAULT_OPS_TIMEOUT;
    call =NULL;
    ssl_check = true;
    userdata = NULL;
    _redirection = true;
}


//
void RequestParams::setAuthentificationCallback(void * _userdata, davix_auth_callback _call){
    call = _call;
    userdata = _userdata;
}


davix_auth_callback RequestParams::getAuthentificationCallbackFunction(){
    return call;
}

void* RequestParams::getAuthentificationCallbackData(){
    return userdata;
}


void RequestParams::setConnexionTimeout(struct timespec *conn_timeout1){
    timespec_copy(&(this->connexion_timeout),conn_timeout1);
}

void RequestParams::setOperationTimeout(struct timespec *ops_timeout1){
    timespec_copy(&(this->ops_timeout), ops_timeout1);
}

const struct timespec* RequestParams::getConnexionTimeout() const {
    return &connexion_timeout;
}

const struct timespec* RequestParams::getOperationTimeout() const {
    return &ops_timeout;
}

void RequestParams::setTransparentRedirectionSupport(bool redirection){
    this->_redirection = redirection;
}


bool RequestParams::getTransparentRedirectionSupport() const{
    return this->_redirection;
}


} // namespace Davix



DAVIX_C_DECL_BEGIN

int davix_params_set_auth_callback(davix_params_t params, davix_auth_callback call, void* userdata, davix_error_t* err){
    g_return_val_if_fail(params != NULL, -1);
    Davix::RequestParams* p = (Davix::RequestParams*)(params);
    p->setAuthentificationCallback(userdata, call);
    return 0;
}


int davix_params_set_ssl_check(davix_params_t params, gboolean ssl_check, davix_error_t* err){
    g_return_val_if_fail(params != NULL, -1);
    Davix::RequestParams* p = (Davix::RequestParams*)(params);
    p->setSSLCAcheck(ssl_check);
    return 0;
}


davix_params_t davix_params_new(){
    return (struct davix_request_params*) new Davix::RequestParams();
}

davix_params_t davix_params_copy(davix_params_t p){
    return (struct davix_request_params*) new Davix::RequestParams(*(Davix::RequestParams*) p);
}

void davix_params_free(davix_params_t p){
    if(p){
        delete ((Davix::RequestParams*) p);
    }
}

DAVIX_C_DECL_END
