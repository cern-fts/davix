#ifndef DAVIX_H
#define DAVIX_H

#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <status/davix_error.h>
#include <davix_types.h>



///
///  @file davix.h
///  @author Devresse Adrien
///
///  @brief API of Davix C
///
///  Davix is a high level HTTP/Webdav library for file management purpose.
///



// Davix POSIX-like API
#include <posix/davix_posix.h>

// davix uri parser
#include <davixuri.h>

DAVIX_C_DECL_BEGIN



///
///  @brief create a new davix session handle
///
///   create a davix sessions handle, session handle can be used to configure and execute davix request
///   @param err : Gerror error report system, in case of failure
///   @return davix_sess_t : session handle or NULL pointer if error
///
davix_sess_t davix_context_new(davix_error_t* err);

///
///   clone a davix context object
///
davix_sess_t davix_context_copy(davix_sess_t sess);


///
/// free a davix context object
void davix_context_free(davix_sess_t sess);




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
///  setup the authorization callback for the current parameter handle
///  This authorisation callback will be called each time that the associated request will need an authentification
///
int davix_params_set_auth_callback(davix_params_t params, davix_auth_callback call, void* userdata, davix_error_t* err);

///
///  disable or enable the validity check of the serveur side credential
///
int davix_params_set_ssl_check(davix_params_t params, gboolean ssl_check, davix_error_t* err);


///
///  Authenficiation callback specific parameters
///
int davix_auth_set_pkcs12_cli_cert(davix_auth_t token, const char* filename_pkcs, const char* passwd, davix_error_t* err);

int davix_auth_set_login_passwd(davix_auth_t token, const char* login, const char* passwd, davix_error_t* err);


DAVIX_C_DECL_END

#endif // DAVIX_H
