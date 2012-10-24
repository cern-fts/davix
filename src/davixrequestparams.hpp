#pragma once
#ifndef DAVIX_REQUESTPARAMS_H
#define DAVIX_REQUESTPARAMS_H

#include "global_def.hpp"



/**
  @file davixrequestparams.hpp
  @author Devresse Adrien

  @brief C++ Davix configuration API
*/


namespace Davix {

class RequestParamsInternal;

class RequestParams
{
public:
    RequestParams();
    RequestParams(const RequestParams & params);
    RequestParams(const RequestParams* params);


    virtual ~RequestParams();


    ///  disable the certificate authority validity check for the https request
    inline void setSSLCAcheck(bool chk){
        ssl_check = chk;
    }

    inline bool getSSLCACheck() const{
        return ssl_check;
    }



    /// set the authentification callback for the associated Request or set of request
    /// userdata will be passed as argument inside the callback
    /// @param userdata : user data handler
    /// @param auth_cb : authentification function
    void setAuthentificationCallback(void * userdata, davix_auth_callback call);

    /// return the current authentification callback
    /// DEFAULT : NULL
    davix_auth_callback getAuthentificationCallbackFunction();

    /// return the current user data
    /// DEFAULT : NULL
    void* getAuthentificationCallbackData();

    /// define the connexion timeout
    /// DEFAULT : 180s
    void setConnexionTimeout(struct timespec* conn_timeout);

    /// get the current connexion timeout
    const struct timespec * getConnexionTimeout()  const;


    /// define the maximum execution time for a davix request
    /// DEFAULT : infinite
    void setOperationTimeout(struct timespec* cops_timeout);

    /// get the maximum execution time for a davix request
    /// DEFAULT : infinite
    const struct timespec * getOperationTimeout()const;


    /// enable or disable transparent redirection support
    /// DEFAULT : enabled
    void setTransparentRedirectionSupport(bool redirection);

    bool getTransparentRedirectionSupport() const;

private:
    struct timespec ops_timeout;
    struct timespec connexion_timeout;
    bool ssl_check;
    bool _redirection;

    // auth info
    void* userdata;
    davix_auth_callback call;
    //
    void _init();
    // dptr
    RequestParamsInternal* d_ptr;


    static void copy(RequestParams & dest, const RequestParams & params);
};


} // namespace Davix

#endif // DAVIX_REQUESTPARAMS_H
