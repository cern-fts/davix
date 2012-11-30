#include <params/davixrequestparams.hpp>
#include <libs/time_utils.h>


namespace Davix {



const char * default_agent = "libdavix/0.0.9";


#define SESSION_FLAG_KEEP_ALIVE 0x01

struct RequestParamsInternal{
    RequestParamsInternal() :
        _ssl_check(true),
        _redirection(true),
        _userdata(NULL),
        _call(NULL),
        ops_timeout(),
        connexion_timeout(),
        agent_string(default_agent),
        _proto(DAVIX_PROTOCOL_WEBDAV),
        _session_flag(SESSION_FLAG_KEEP_ALIVE)
    {
        timespec_clear(&connexion_timeout);
        timespec_clear(&ops_timeout);
        connexion_timeout.tv_sec = DAVIX_DEFAULT_CONN_TIMEOUT;
        ops_timeout.tv_sec = DAVIX_DEFAULT_OPS_TIMEOUT;
    }

    virtual ~RequestParamsInternal(){

    }
    RequestParamsInternal(const RequestParamsInternal & param_private):
        _ssl_check(param_private._ssl_check),
        _redirection(param_private._redirection),
        _userdata(param_private._userdata),
        _call(param_private._call),
        ops_timeout(),
        connexion_timeout(),
        agent_string(param_private.agent_string),
        _proto(param_private._proto),
        _session_flag(param_private._session_flag){

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

    // user agent
    std::string agent_string;

    // proto
    davix_request_protocol_t  _proto;

    // session flag
    int _session_flag;
private:
    RequestParamsInternal & operator=(const RequestParamsInternal & params);
};


RequestParams::RequestParams() :
    d_ptr(new RequestParamsInternal())
{

}

RequestParams::RequestParams(const RequestParams& params) :
    d_ptr(new RequestParamsInternal(*(params.d_ptr))){

}




RequestParams::~RequestParams(){
   delete d_ptr;
}

RequestParams::RequestParams(const RequestParams* params) :
    d_ptr( ((params)?(new RequestParamsInternal(*(params->d_ptr))):(new RequestParamsInternal())) ){

}


RequestParams & RequestParams::operator=(const RequestParams & orig){
    if(d_ptr != orig.d_ptr)
        delete d_ptr;
    d_ptr = new RequestParamsInternal(*(orig.d_ptr));
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


davix_auth_callback RequestParams::getAuthentificationCallbackFunction() const{
    return d_ptr->_call;
}

void* RequestParams::getAuthentificationCallbackData() const{
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

const std::string & RequestParams::getUserAgent() const{
    return d_ptr->agent_string;
}

void RequestParams::setUserAgent(const std::string &user_agent){
    d_ptr->agent_string = user_agent;
}


const davix_request_protocol_t RequestParams::getProtocol() const {
    return d_ptr->_proto;
}

void RequestParams::setProtocol(const davix_request_protocol_t proto){
    d_ptr->_proto = proto;
}

void RequestParams::setKeepAlive(const bool keep_alive_flag){
    if(keep_alive_flag)
        d_ptr->_session_flag |= SESSION_FLAG_KEEP_ALIVE;
    else
        d_ptr->_session_flag &= ~(SESSION_FLAG_KEEP_ALIVE);
}


const bool RequestParams::getKeepAlive() const{
    return d_ptr->_session_flag & SESSION_FLAG_KEEP_ALIVE;
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
