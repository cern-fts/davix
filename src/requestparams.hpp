#pragma once
#ifndef DAVIX_REQUESTPARAMS_H
#define DAVIX_REQUESTPARAMS_H

#include "global_def.hpp"

namespace Davix {


class RequestParams
{
public:
    RequestParams();
    RequestParams(const RequestParams & params);

    /**
      disable the certificate authority validity check for the https request
    */
    virtual void set_ssl_ca_check(bool chk);  // throw nothing

    /**
      authentification callback for right management
    */
    virtual void set_authentification_controller(void * userdata, davix_auth_callback call);
    /**
      define the connexion timeout in seconds
    */
    virtual void set_connexion_timeout(unsigned long timeout);

    /**
      define the operation timeout in seconds
    */
    virtual void set_operation_timeout(unsigned long timeout);

protected:
    unsigned long ops_timeout;
    unsigned long connexion_timeout;
    bool ssl_check;

    // auth callback
    davix_auth_callback call;
    void* userdata;

    friend class NeonRequest;
};

} // namespace Davix

#endif // DAVIX_REQUESTPARAMS_H
