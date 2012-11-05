#include <davixrequestparams.hpp>
#include <libs/time_utils.h>


namespace Davix {


struct RequestParamsInternal{
    RequestParamsInternal(){
     _ssl_check = true;
     _redirection = true;
     _call = NULL;
     _userdata = NULL;
     timespec_clear(&connexion_timeout);
     timespec_clear(&ops_timeout);
     connexion_timeout.tv_sec = DAVIX_DEFAULT_CONN_TIMEOUT;
     ops_timeout.tv_sec = DAVIX_DEFAULT_OPS_TIMEOUT;
    }
    virtual ~RequestParamsInternal(){

    }
    RequestParamsInternal(const RequestParamsInternal & param_private){
        _ssl_check = param_private._ssl_check;
        _redirection = param_private._redirection;
        _userdata = param_private._userdata;
        _call = param_private._call;
        timespec_copy(&(connexion_timeout), &(param_private.connexion_timeout));
        timespec_copy(&(ops_timeout), &(param_private.ops_timeout));
    }
    bool _ssl_check; // ssl CA check
    bool _redirection; // redirection support

    // auth info
    void* _userdata;
    davix_auth_callback _call;

    // timeout management
    struct timespec ops_timeout;
    struct timespec connexion_timeout;
};


RequestParams::RequestParams()
{
    d_ptr= NULL;
    _init();
}

RequestParams::RequestParams(const RequestParams& params){
    copy(this, &params);
}




RequestParams::~RequestParams(){
   delete d_ptr;
}

RequestParams::RequestParams(const RequestParams* params){
    if(params){
        copy(this, params);
    }else{
        _init();
    }
}

void RequestParams::copy(RequestParams *dest, const RequestParams *params){

    dest->d_ptr = new RequestParamsInternal(*(params->d_ptr));
}


void RequestParams::_init(){

    d_ptr = new RequestParamsInternal();
}

RequestParams & RequestParams::operator=(const RequestParams & orig){
    if(d_ptr != orig.d_ptr)
        delete d_ptr;
    copy(this, &orig);
    return *this;
}


bool RequestParams::getSSLCACheck() const{
    return d_ptr->_ssl_check;
}

void RequestParams::setSSLCAcheck(bool chk){
    d_ptr->_ssl_check = chk;
}

//
void RequestParams::setAuthentificationCallback(void * _userdata, davix_auth_callback _call){
    d_ptr->_call = _call;
    d_ptr->_userdata = _userdata;
}


davix_auth_callback RequestParams::getAuthentificationCallbackFunction(){
    return d_ptr->_call;
}

void* RequestParams::getAuthentificationCallbackData(){
    return d_ptr->_userdata;
}


void RequestParams::setConnexionTimeout(struct timespec *conn_timeout1){
    timespec_copy(&(d_ptr->connexion_timeout),conn_timeout1);
}

void RequestParams::setOperationTimeout(struct timespec *ops_timeout1){
    timespec_copy(&(d_ptr->ops_timeout), ops_timeout1);
}

const struct timespec* RequestParams::getConnexionTimeout() const {
    return &d_ptr->connexion_timeout;
}

const struct timespec* RequestParams::getOperationTimeout() const {
    return &d_ptr->ops_timeout;
}

void RequestParams::setTransparentRedirectionSupport(bool redirection){
    d_ptr->_redirection = redirection;
}


bool RequestParams::getTransparentRedirectionSupport() const{
    return d_ptr->_redirection;
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
