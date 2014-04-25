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

#include <utils/davix_logger_internal.hpp>
#include <params/davixrequestparams.hpp>
#include <libs/time_utils.h>



namespace Davix {



const char * default_agent = "libdavix/" DAVIX_VERSION;
volatile int state_value =0;

inline int get_requeste_uid(){
    state_value +=1;
    return state_value;
}


#define SESSION_FLAG_KEEP_ALIVE 0x01

struct RequestParamsInternal{
    RequestParamsInternal() :
        _ssl_check(true),
        _redirection(true),
        _ca_path(),
        _cli_cert(),
        _idlogpass(),
        _callb(NULL),
        _callb_userdata(NULL),
        _call_loginpswwd(NULL),
        _call_loginpswwd_userdata(NULL),
        _aws_cred(),
        ops_timeout(),
        connexion_timeout(),
        agent_string(default_agent),
        _proto(RequestProtocol::Auto),
        _metalink_mode(MetalinkMode::Auto),
        _customhdr(),
        _session_flag(SESSION_FLAG_KEEP_ALIVE),
        _state_uid(get_requeste_uid())
    {
        timespec_clear(&connexion_timeout);
        timespec_clear(&ops_timeout);
        connexion_timeout.tv_sec = DAVIX_DEFAULT_CONN_TIMEOUT;
        ops_timeout.tv_sec = DAVIX_DEFAULT_OPS_TIMEOUT;
    }

    virtual ~RequestParamsInternal(){
        if(_idlogpass.second.empty() == false){
            // eradicate password from memory
            for(std::string::iterator it = _idlogpass.second.begin(); it < _idlogpass.second.end(); ++it)
                *it = 'a';
        }

    }
    RequestParamsInternal(const RequestParamsInternal & param_private):
        _ssl_check(param_private._ssl_check),
        _redirection(param_private._redirection),
        _ca_path(param_private._ca_path),
        _cli_cert(param_private._cli_cert),
        _idlogpass(param_private._idlogpass),
        _callb(param_private._callb),
        _callb_userdata(param_private._callb_userdata),
        _call_loginpswwd(param_private._call_loginpswwd),
        _call_loginpswwd_userdata(param_private._call_loginpswwd_userdata),
        _aws_cred(param_private._aws_cred),
        ops_timeout(),
        connexion_timeout(),
        agent_string(param_private.agent_string),
        _proto(param_private._proto),
        _metalink_mode(param_private._metalink_mode),
        _customhdr(param_private._customhdr),
        _session_flag(param_private._session_flag),
        _state_uid(param_private._state_uid){

        timespec_copy(&(connexion_timeout), &(param_private.connexion_timeout));
        timespec_copy(&(ops_timeout), &(param_private.ops_timeout));
    }
    bool _ssl_check; // ssl CA check
    bool _redirection; // redirection support

    // CA management
    std::vector<std::string> _ca_path;

    // auth info
    X509Credential _cli_cert;
    std::pair<std::string,std::string> _idlogpass;
    authCallbackClientCertX509 _callb;
    void* _callb_userdata;
    authCallbackLoginPasswordBasic _call_loginpswwd;
    void* _call_loginpswwd_userdata;
    std::pair<AwsSecretKey, AwsAccessKey> _aws_cred;

    // timeout management
    struct timespec ops_timeout;
    struct timespec connexion_timeout;

    // user agent
    std::string agent_string;

    // proto
    RequestProtocol::Protocol  _proto;
    MetalinkMode::MetalinkMode _metalink_mode;

    // additional custom header lines
    HeaderVec _customhdr;

    // session flag
    int _session_flag;

    // ssl state value, a state uid is used to check if two copy of a requestParam struct are equal
    int _state_uid;


    // method
    inline void regenerateStateUid(){
        _state_uid = get_requeste_uid();
    }

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
    d_ptr->regenerateStateUid();
    d_ptr->_ssl_check = chk;
}


void RequestParams::setClientCertX509(const X509Credential & cli_cert){
    d_ptr->regenerateStateUid();
    d_ptr->_cli_cert = cli_cert;
}

void RequestParams::setClientLoginPassword(const std::string & login, const std::string & password){
    d_ptr->regenerateStateUid();
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
    d_ptr->regenerateStateUid();
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
    d_ptr->regenerateStateUid();
    d_ptr->_call_loginpswwd = callback;
    d_ptr->_call_loginpswwd_userdata = userdata;
}

/// return the current client side callback for authentification with the current user data
std::pair<authCallbackLoginPasswordBasic,void*> RequestParams::getClientLoginPasswordCallback() const{
    return std::pair<authCallbackLoginPasswordBasic,void*>(d_ptr->_call_loginpswwd, d_ptr->_call_loginpswwd_userdata);
}


void RequestParams::setAwsAuthorizationKeys(const std::string &secret_key, const std::string &access_key){
    d_ptr->_aws_cred = std::pair<AwsSecretKey, AwsAccessKey>(secret_key,access_key);
}

const std::pair<AwsSecretKey, AwsAccessKey> & RequestParams::getAwsAutorizationKeys() const{
    return d_ptr->_aws_cred;
}

void RequestParams::addCertificateAuthorityPath(const std::string &path){
    d_ptr->regenerateStateUid();
    d_ptr->_ca_path.push_back(path);
}

const std::vector<std::string> & RequestParams::listCertificateAuthorityPath() const{
    return d_ptr->_ca_path;
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
    d_ptr->regenerateStateUid();
    d_ptr->_redirection = redirection;
}


bool RequestParams::getTransparentRedirectionSupport() const{
    return d_ptr->_redirection;
}

const std::string & RequestParams::getUserAgent() const{
    return d_ptr->agent_string;
}

void RequestParams::setUserAgent(const std::string &user_agent){
    d_ptr->regenerateStateUid();
    d_ptr->agent_string = user_agent;
}


RequestProtocol::Protocol RequestParams::getProtocol() const {
    return d_ptr->_proto;
}

void RequestParams::setProtocol(const RequestProtocol::Protocol proto){
    d_ptr->_proto = proto;
}

const MetalinkMode::MetalinkMode RequestParams::getMetalinkMode() const{
    return d_ptr->_metalink_mode;
}

void RequestParams::setMetalinkMode(const MetalinkMode::MetalinkMode mode){
    d_ptr->_metalink_mode = mode;
}


void RequestParams::setKeepAlive(const bool keep_alive_flag){
    d_ptr->regenerateStateUid();
    if(keep_alive_flag)
        d_ptr->_session_flag |= SESSION_FLAG_KEEP_ALIVE;
    else
        d_ptr->_session_flag &= ~(SESSION_FLAG_KEEP_ALIVE);
}


bool RequestParams::getKeepAlive() const{
    return d_ptr->_session_flag & SESSION_FLAG_KEEP_ALIVE;
}



void RequestParams::addHeader(const std::string &key, const std::string &val) {

  d_ptr->_customhdr.push_back( std::pair<std::string,std::string>(key, val) );


}
const HeaderVec & RequestParams::getHeaders() const{
  return d_ptr->_customhdr;
}

// suppress useless warning
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
void* RequestParams::getParmState() const{

    return (void*) (d_ptr->_state_uid);
}

void RequestParams::swap(RequestParams & p){
    std::swap(d_ptr, p.d_ptr);
}


} // namespace Davix


