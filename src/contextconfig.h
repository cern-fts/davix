#pragma once
#ifndef CONTEXTCONFIG_H
#define CONTEXTCONFIG_H

#include <ctime>
#include <davix_types.h>
#include <global_def.hpp>

namespace Davix{

/// Context configuration handler
class ContextConfig
{
public:
    ContextConfig();
    ContextConfig(const ContextConfig & conf);
    virtual ~ContextConfig();


   /// disable the certificate authority validity check for the https request
   void setSSLCACheck(bool chk);  // throw nothing


    /// setup a authentification callback for the associated context
    void setAuthCallback(void * userdata, davix_auth_callback call);

    /// setup a connexion timeout
    void setConnexionTimeout(struct timespec* conn_timeout);

    /**
      define the operation timeout in seconds
    */
    void setOperationTimeout(struct timespec* cops_timeout);

protected:
    struct timespec ops_timeout;
    struct timespec connexion_timeout;
    bool ssl_check;

    // auth callback
    davix_auth_callback call;
    void* userdata;
};

}

#endif // CONTEXTCONFIG_H
