#pragma once
#ifndef DAVIX_REQUESTPARAMS_H
#define DAVIX_REQUESTPARAMS_H

#include "global_def.hpp"

namespace Davix {


struct RequestParams
{
public:
    RequestParams();
    RequestParams(const RequestParams & params);

    virtual ~RequestParams();

    /**
      disable the certificate authority validity check for the https request
    */
    void set_ssl_ca_check(bool chk);

    /**
      authentification callback for right management
    */
    void set_authentification_controller(void * userdata, davix_auth_callback call);
    /**
      define the connexion timeout in seconds
    */
    void set_connexion_timeout(unsigned long timeout);

    /**
      define the operation timeout in seconds
    */
    void set_operation_timeout(unsigned long timeout);

    unsigned long ops_timeout;
    unsigned long connexion_timeout;
    bool ssl_check;

    // auth callback
    davix_auth_callback call;
    void* userdata;
};


} // namespace Davix

#endif // DAVIX_REQUESTPARAMS_H
