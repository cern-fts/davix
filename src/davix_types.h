#ifndef DAVIX_TYPES_H
#define DAVIX_TYPES_H

#include <glib.h>

/**
  Davix Error system

  if Glib::Error-> code < 1000
        -> errno value
  else
        -> internal Davix value

  Common Error code associated with code field in Glib::Error or GError
*/
#define DAVIX_ERROR_ENOTSUPPORT EPROTONOSUPPORT /*< functionality not supported */
#define DAVIX_ERROR_NOPASSWD 1000                /*< password required but not provided in the authentification callback */
#define DAVIX_ERROR_BADPASSWD 1001                /*< Bad login/password, failure */

typedef void* davix_sess_t;
typedef void DAVIX_DIR;
typedef struct davix_auth_st* davix_auth_t;
typedef struct davix_request_params* davix_params_t;

/**
  @brief authentification type requested
*/
typedef enum _AUTH_TYPE{
    DAVIX_LOGIN_PASSWORD,       /* login / password */
    DAVIX_CLI_CERT_PKCS12,      /* PKCS12 client credential */
    DAVIX_INVALID_AUTH_TYPE     /* limit type */
} AUTH_TYPE;

typedef struct{
    char * dnames;              /**< Optional : DN of the allowed authorities */
    AUTH_TYPE auth;             /**< required authentification ( login/password / credential, etc ... ) */
} davix_auth_info_t;

/**
  This callback is called when a authentification is needed
  It allows to call the authentification function with the given token
  @return this callback should return 0 on success, -1 and err is set if major problem occures
*/
typedef int (*davix_auth_callback)(davix_auth_t token, const davix_auth_info_t* t, void* userdata, GError** err);




#endif // DAVIX_TYPES_H
