#pragma once
#ifndef DAVIXREQUESTPARAMS_H
#define DAVIXREQUESTPARAMS_H

/**
  @file davixrequestparams.h
  @author Devresse Adrien

  @brief C Davix configuration API
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

#include <davix_types.h>
#include <auth/davixauth.h>

DAVIX_C_DECL_BEGIN

//
// Params management functions
//

///
/// create a davix parameter container with default parameter values
/// a davix parameter container can be used to setup advanced settings to any davix request
///
/// need to be free with @ref davix_params_free
///
davix_params_t davix_params_new();

///
/// clone a davix parameter object
///
davix_params_t davix_params_copy(davix_params_t src);

///
///  free a davix parameter container
///
void davix_params_free(davix_params_t p);


///
///  disable or enable the validity check of the serveur side credential
///
int davix_params_set_ssl_check(davix_params_t params, bool ssl_check, davix_error_t* err);

/// get the current value of the http keep alive option
bool davix_params_get_keep_alive(davix_params_t params);


/// enable or disable http keep alive
void davix_params_set_keep_alive(davix_params_t params, bool keep_alive);


/// protocol used by the  request
/// DAVIX_PROTOCOL_WEBDAV : HTTP + Webdav
/// DAVIX_PROTOCOL_HTTP :  HTTP only
davix_request_protocol_t davix_params_get_protocol(davix_params_t params);


/// set protocol used by the  request
void davix_params_set_protocol(davix_params_t params, davix_request_protocol_t protocol);


/// enable or disable transparent redirection support
/// In the transparent redirection mode,
/// davix follows the HTTP redirection automatically
/// default : enabled
void davix_params_set_trans_redirect(davix_params_t params, bool redirection);

/// return true if the transparent redirection mode is enabled
bool davix_params_get_trans_redirect(davix_params_t params);

/// set the user agent for the associated request
void davix_params_set_user_agent(davix_params_t params, const char* user_agent);

/// get the current user agent string
const char* davix_params_get_user_agent(davix_params_t params);


/// define the connexion timeout
/// conn_timeout is a relative time
/// DEFAULT : 180s
void davix_params_set_conn_timeout(davix_params_t params, unsigned int timeout);

/// get the current connexion timeout
unsigned int davix_params_get_conn_timeout(davix_params_t params);


/// define the maximum execution time for a davix request
/// ops_timeout is a relative time
/// DEFAULT : infinite
void davix_params_set_ops_timeout(davix_params_t params, unsigned int timeout);

/// get the maximum execution time for a davix request
/// DEFAULT : infinite
unsigned int davix_params_get_ops_timeout(davix_params_t params);

/// set login/password for HTTP Authentication
void davix_params_set_login_passwd(davix_params_t params, const char* login, const char*  password);


/// set a X509 credential for a simple client authentication
/// this function overwrite \ref setClientCertCallbackX509
void davix_params_set_client_cert_X509(davix_params_t params, davix_x509_cert_t cred);

/// get the current client side credential
davix_x509_cert_t  davix_params_get_client_cert_X509(davix_params_t params);

DAVIX_C_DECL_END

#endif // DAVIXREQUESTPARAMS_H
