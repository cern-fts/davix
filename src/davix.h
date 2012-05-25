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
  POSIX and file oriented API for Webdav/HTTP file management
  davix support :
  - redirection
  - SSL credentials
  - Login/password authentification
  - Grid-style configuration ( GSI, delegation )
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

#ifdef __cplusplus
}
#endif

#endif // DAVIX_H
