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
  @brief POSIX-like API for Webdav/HTTP file access

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

/**
  @brief execute a POSIX stat query on a given url, required webdav endpoint

  Try to do a POSIX stat request on a given webdav endpoint
  @param sess : davix session handle
  @param url: url of the webdav point
  @param st : stat structure
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_stat(davix_sess_t sess, const char* url, struct stat * st, GError** err);

/**
  to use from authentification call_back
  allow to authentificate with a clicert pkcs12
*/
int davix_set_pkcs12_auth(davix_auth_t token, const char* filename_pkcs12,const char* passwd, GError** err);

/**
  to use from authentification call_back
  allow to authentificate with a login/password
*/
int davix_set_login_passwd_auth(davix_auth_t token, const char* login, const char* passwd, GError** err);


/**
  C API, specifie the callback for client authentification
*/
int davix_set_auth_callback(davix_sess_t sess, davix_auth_callback call, void* userdata, GError** err);

/**
  C API, enable or disable the CA check for X509 credential while the authentification
  Passing false to this function is similar to the -k or --insecure option of curl
  true by default
*/
int davix_set_ssl_check(davix_sess_t sess, gboolean ssl_check, GError** err);

#ifdef __cplusplus
}
#endif

#endif // DAVIX_H
