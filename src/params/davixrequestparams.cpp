#include <auth/davixauth.h>
#include <params/davixrequestparams.hpp>
#include <libs/time_utils.h>


namespace Davix {



const char * default_agent = "libdavix/0.0.9";


#define SESSION_FLAG_KEEP_ALIVE 0x01

struct RequestParamsInternal{
    RequestParamsInternal() :
        _ssl_check(true),
        _redirection(true),
        _cli_cert(),
        _idlogpass(),
        _callb(NULL),
        _callb_userdata(NULL),
        _call_loginpswwd(NULL),
        _call_loginpswwd_userdata(NULL),
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
        _cli_cert(param_private._cli_cert),
        _idlogpass(param_private._idlogpass),
        _callb(param_private._callb),
        _callb_userdata(param_private._callb_userdata),
        _call_loginpswwd(param_private._call_loginpswwd),
        _call_loginpswwd_userdata(param_private._call_loginpswwd_userdata),
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
    X509Credential _cli_cert;
    std::pair<std::string,std::string> _idlogpass;
    authCallbackClientCertX509 _callb;
    void* _callb_userdata;
    authCallbackLoginPasswordBasic _call_loginpswwd;
    void* _call_loginpswwd_userdata;

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


void RequestParams::setClientCertX509(const X509Credential & cli_cert){
    d_ptr->_cli_cert = cli_cert;
}

void RequestParams::setClientLoginPassword(const std::string & login, const std::string & password){
    d_ptr->_idlogpass = std::make_pair(login, password);
}

const std::pair<std::string, std::string> & RequestParams::getClientLoginPassword() const{
    return d_ptr->_idlogpass;
}

const X509Credential & RequestParams::getClientCertX509() const{
    return d_ptr->_cli_cert;
}

/// set a callback for X509 client side dynamic authentication
/// this function overwrite \ref setClientCertX509
void RequestParams::setClientCertCallbackX509(authCallbackClientCertX509 callback, void* userdata){
    d_ptr->_callb = callback;
    d_ptr->_callb_userdata = userdata;
}

/// return the current client side callback for authentification with the current user data
std::pair<authCallbackClientCertX509,void*> RequestParams::getClientCertCallbackX509() const{
    return std::pair<authCallbackClientCertX509,void*>(d_ptr->_callb, d_ptr->_callb_userdata);
}

/// set a callback for X509 client side dynamic authentication
/// this function overwrite \ref setClientCertX509
void RequestParams::setClientLoginPasswordCallback(authCallbackLoginPasswordBasic callback, void* userdata){
    d_ptr->_call_loginpswwd = callback;
    d_ptr->_call_loginpswwd_userdata = userdata;
}

/// return the current client side callback for authentification with the current user data
std::pair<authCallbackLoginPasswordBasic,void*> RequestParams::getClientLoginPasswordCallback() const{
    return std::pair<authCallbackLoginPasswordBasic,void*>(d_ptr->_call_loginpswwd, d_ptr->_call_loginpswwd_userdata);
}

void RequestParams::setConnectionTimeout(struct timespec *conn_timeout1){
    timespec_copy(&(d_ptr->connexion_timeout),conn_timeout1);
}

void RequestParams::setOperationTimeout(struct timespec *ops_timeout1){
    timespec_copy(&(d_ptr->ops_timeout), ops_timeout1);
}

const struct timespec* RequestParams::getConnectionTimeout() const {
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


////////////////////////////////////////////////
////////////////////////////////////////////////
///// C bindings
////////////////////////////////////////////////
////////////////////////////////////////////////


DAVIX_C_DECL_BEGIN




int davix_params_set_ssl_check(davix_params_t params, bool ssl_check, davix_error_t* err){
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

bool davix_params_get_keep_alive(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getKeepAlive();
}


/// enable or disable http keep alive
void davix_params_set_keep_alive(davix_params_t params, bool keep_alive){
    g_assert(params);
    ((Davix::RequestParams*) params)->setKeepAlive(keep_alive);
}

void davix_params_set_protocol(davix_params_t params, davix_request_protocol_t protocol){
    g_assert(params);
    ((Davix::RequestParams*) params)->setProtocol(protocol);
}

davix_request_protocol_t davix_params_get_protocol(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getProtocol();
}


void davix_params_set_trans_redirect(davix_params_t params, bool redirection){
    g_assert(params);
    ((Davix::RequestParams*) params)->setTransparentRedirectionSupport(redirection);
}

bool davix_params_get_trans_redirect(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getTransparentRedirectionSupport();
}

void davix_params_set_user_agent(davix_params_t params, const char* user_agent){
    g_assert(params && user_agent);
    ((Davix::RequestParams*) params)->setUserAgent(user_agent);
}


const char* davix_params_get_user_agent(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getUserAgent().c_str();
}


void davix_params_set_conn_timeout(davix_params_t params, unsigned int timeout){
    g_assert(params);
    struct timespec t;
    t.tv_sec = timeout;
    t.tv_nsec =0;
    return ((Davix::RequestParams*) params)->setConnectionTimeout(&t);
}

unsigned int davix_params_get_conn_timeout(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getConnectionTimeout()->tv_sec;
}



void davix_params_set_ops_timeout(davix_params_t params, unsigned int timeout){
    g_assert(params);
    struct timespec t;
    t.tv_sec = timeout;
    t.tv_nsec =0;
    return ((Davix::RequestParams*) params)->setOperationTimeout(&t);
}

unsigned int davix_params_get_ops_timeout(davix_params_t params){
    g_assert(params);
    return ((Davix::RequestParams*) params)->getOperationTimeout()->tv_sec;
}


void davix_params_set_client_cert_X509(davix_params_t params, davix_x509_cert_t cred){
    g_assert(params && cred);
    ((Davix::RequestParams*) params)->setClientCertX509(*(Davix::X509Credential*) cred);
}


davix_x509_cert_t  davix_params_get_client_cert_X509(davix_params_t params){
    g_assert(params);
    return (davix_x509_cert_t) &((Davix::RequestParams*) params)->getClientCertX509();
}

/// set login/password for HTTP Authentication
void davix_params_set_login_passwd(davix_params_t params, const char* login, const char*  password){
    g_assert(params && login && password);
    ((Davix::RequestParams*) params)->setClientLoginPassword(login, password);
}


DAVIX_C_DECL_END
