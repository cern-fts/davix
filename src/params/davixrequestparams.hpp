#pragma once
#ifndef DAVIX_REQUESTPARAMS_HPP
#define DAVIX_REQUESTPARAMS_HPP

#include "global_def.hpp"
#include <auth/davixauth.hpp>


/**
  @file davixrequestparams.hpp
  @author Devresse Adrien

  @brief C++ Davix configuration API
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif



namespace Davix {

class RequestParamsInternal;




///
/// @class RequestParams
/// @brief Main container for HTTP/WebDAV request options
///
/// RequestParams hold the davix request options :
/// authentification parameters, timeouts, user-agents,...
/// A Requestparams object can be shared between several Request
class RequestParams
{
public:
    RequestParams();
    RequestParams(const RequestParams & params);
    RequestParams(const RequestParams* params);

    virtual ~RequestParams();


    ///  disable the certificate authority validity check for the https request
    void setSSLCAcheck(bool chk);

    /// return the SSL Certificate authority validity check
    bool getSSLCACheck() const;

    /// set a X509 credential for a simple client authentication
    /// this function overwrite \ref setClientCertCallbackX509
    void setClientCertX509(const X509Credential & cli_cert);

    /// get the current client side credential
    const X509Credential &  getClientCertX509() const;

    /// set login/password for HTTP Authentication
    void setClientLoginPassword(const std::string & login, const std::string & password);

    /// get login/password for HTTP Authentication
    const std::pair<std::string,std::string> & getClientLoginPassword() const;


    /// set a callback for X509 client side dynamic authentication
    /// this function overwrite \ref setClientCertX509
    void setClientCertCallbackX509(authCallbackClientCertX509 callback, void* userdata);

    /// return the current client side callback for authentification with the associated user data
    std::pair<authCallbackClientCertX509,void*> getClientCertCallbackX509() const;

    /// set a callback for basic login/password http authentification
    /// this function overwrite \ref setClientLoginPassword
    void setClientLoginPasswordCallback(authCallbackLoginPasswordBasic callback, void* userdata);

    /// return the current login/password callback and the associated user data
    std::pair<authCallbackLoginPasswordBasic,void*> getClientLoginPasswordCallback() const;

    /// define the connexion timeout
    /// conn_timeout is a relative time
    /// DEFAULT : 180s
    void setConnectionTimeout(struct timespec* conn_timeout);

    /// get the current connexion timeout
    const struct timespec * getConnectionTimeout()  const;


    /// define the maximum execution time for a davix request
    /// ops_timeout is a relative time
    /// DEFAULT : infinite
    void setOperationTimeout(struct timespec* ops_timeout);

    /// get the maximum execution time for a davix request
    /// DEFAULT : infinite
    const struct timespec * getOperationTimeout()const;


    /// enable or disable transparent redirection support
    /// In the transparent redirection mode,
    /// davix follows the HTTP redirection automatically
    /// DEFAULT : enabled
    void setTransparentRedirectionSupport(bool redirection);

    /// return true if the transparent redirection mode is enabled
    bool getTransparentRedirectionSupport() const;

    /// set the user agent for the associated request
    void setUserAgent(const std::string & user_agent);

    /// get the current user agent string
    const std::string & getUserAgent() const;

    /// set the request protocol ( ex : Webdav, Http-only )
    void setProtocol(const davix_request_protocol_t proto);

    /// get the current value of the request protocol
    const davix_request_protocol_t getProtocol() const;

    /// set the keep alive value of the associated session
    void setKeepAlive(const bool keep_alive_flag);

    /// get the keep alive value of this request params
    const bool getKeepAlive() const;


    RequestParams & operator=(const RequestParams & _p);
private:

   // dptr
    RequestParamsInternal* d_ptr;

};


} // namespace Davix

#endif // DAVIX_REQUESTPARAMS_HPP
