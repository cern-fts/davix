#pragma once
#ifndef DAVIX_REQUESTPARAMS_H
#define DAVIX_REQUESTPARAMS_H

#include "global_def.hpp"


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




    /// set the authentification callback for the associated Request or set of request
    /// userdata will be passed as argument inside the callback
    /// @param userdata : user data handler
    /// @param auth_cb : authentification function
    void setAuthentificationCallback(void * userdata, davix_auth_callback call);

    /// return the current authentification callback
    /// DEFAULT : NULL
    davix_auth_callback getAuthentificationCallbackFunction() const;

    /// return the current user data
    /// DEFAULT : NULL
    void* getAuthentificationCallbackData() const;

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

#endif // DAVIX_REQUESTPARAMS_H
