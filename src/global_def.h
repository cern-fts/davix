#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include <iostream>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <logging/logger.h>

namespace Davix{

typedef void DAVIX_DIR;

inline void davix_log_debug(const gchar *format,
                            ...){
    va_list va;
    va_start(va, format);
    g_loggerv("DAVIX", G_LOG_LEVEL_DEBUG ,
              format, va);
    va_end(va);
}

inline void davix_log_warning(const gchar *format,
                            ...){
    va_list va;
    va_start(va, format);
    g_loggerv("DAVIX", G_LOG_LEVEL_WARNING ,
              format, va);
    va_end(va);
}

typedef enum _REQUEST_TYPE{
    HTTP,
    HTTPS,
    DAVIX_FILE
} RequestType;


typedef enum _Auth_code{
    DAVIX_AUTH_SUCCESS,
    DAVIX_AUTH_FAILURE,
    DAVIX_AUTH_SKIP
} Auth_code;

/**
  Authentification information required
*/
typedef enum _Auth_type{
    DAVIX_PROXY_FULL_PEM,   // PEM proxy certificate like described in http://dev.globus.org/wiki/Security/ProxyFileFormat
    DAVIX_CERT_PEM,         // CERT PEM
    DAVIX_KEY_PEM,          // Private KEY PEM
    DAVIX_KEY_PASSWD,       //  Private key password
    DAVIX_LOGIN,
    DAVIX_PASSWD
} Auth_type;

/**
  This callback is called when a given information is needed
  FULL_PEM : a path to a full PEM credential
  CERT_PEM : a path to a public key pem
  KEY_PEM : a path to a private key pem
  KEY_PASSWD : the password requried for the pem decryption ( can be "" if it does not exist )
  LOGIN : login in classical login/auth
  PASSWD : passwd in a classical login/auth
  @return this callback should return 0 on success, 1 if this access is not managed,
*/
typedef  Auth_code (*davix_auth_callback)(Auth_type t, char * data,  void * userdata, GError ** err);


#define DAVIX_GRID_NONE 0x00
#define DAVIX_GRID_SSL_CHECK 0x01           /**< grid_mode : disable the SSL ca checking, usefull for worker node configuration */
#define DAVIX_GRID_AUTO_VOMS 0x02           /**< grid mode : enable the detection of VOMS certificate, enable it if found */
#define DAVIX_GRID_ALL (~(0x00))            /**< enable all the grid mode */


#define DAVIX_BUFFER_SIZE 2048


}

#endif // GLOBAL_DEF_H
