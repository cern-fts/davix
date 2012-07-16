#ifndef DAVIX_H
#define DAVIX_H

#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <davix_types.h>

/**
  @file davix.h
  @author Devresse Adrien
  @brief High level Http/Webdav interface, with file operations

  Davix support :
  - redirection on every calls ( cluster support )
  - SSL client side credentials
  - Login/password authentification
  - Third party transfer
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
  @brief create a new davix session handle

  create a davix sessions handle, session handle can be used to configure and execute davix request
  @param err : Gerror error report system, in case of failure
  @return davix_sess_t : session handle or NULL poitner if error
  */
davix_sess_t davix_session_new(GError ** err);

/**
  enable/disable grid mode, grid mode enable GSI/voms usage and credential delegation

*/

/**
  release a davix session handle
*/
void davix_session_free(davix_sess_t sess);




//
// Params management functions
//

/**
 * create a new parameter container with default parameter values
 * this parameter container needs to be free with @ref davix_params_free
 */
davix_params_t davix_params_new();

/**
 * copy the davix parameters from src to a new allocated one
 * this copy needs to be free with @ref davix_params_free
 */
davix_params_t davix_params_copy(davix_params_t src);

/**
  free and destruct a parameter container
*/
void davix_params_free(davix_params_t p);

/**
  setup the authorization callback for the current parameter handle
  This authorisation callback will be called each time that the associated request will need an authentification
*/
int davix_params_set_auth_callback(davix_params_t params, davix_auth_callback call, void* userdata, GError** err);

/**
  disable or enable the validity check of the serveur side credential
*/
int davix_params_set_ssl_check(davix_params_t params, gboolean ssl_check, GError** err);


/**
  set the default parameter container to use for the current session
*/
int davix_set_default_session_params(davix_sess_t sess, davix_params_t params, GError ** err);

//
// Authenficiation callback specific parameters
//
int davix_set_pkcs12_auth(davix_auth_t token, const char* filename_pkcs, const char* passwd, GError** err);

int davix_set_login_passwd_auth(davix_auth_t token, const char* login, const char* passwd, GError** err);


//
// POSIX like API
// Need Webdav support
//


/**
  @brief execute a POSIX stat query on a given url, required webdav endpoint

  POSIX-like operation,

  POSIX stat request on a given webdav endpoint
  @param sess : davix session handle
  @param url: url of the webdav point
  @param st : stat structure
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_stat(davix_sess_t sess, const char* url, struct stat * st, GError** err);



//
// Settings management API
//
//

/**
  C API, enable or disable the CA check for X509 credential while the authentification
  Passing false to this function is similar to the -k or --insecure option of curl
  true by default
*/
//int davix_set_ssl_check(davix_sess_t sess, gboolean ssl_check, GError** err);

#ifdef __cplusplus
}
#endif

#endif // DAVIX_H
