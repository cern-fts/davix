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

DAVIX_C_DECL_END

#endif // DAVIXREQUESTPARAMS_H
