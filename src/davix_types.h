#ifndef DAVIX_TYPES_H
#define DAVIX_TYPES_H

#include <glib.h>

/**
  @file davix_types.h
  @author Devresse Adrien


  @brief davix types declarations

  Davix is a file access and file management library on top of HTTP/Webdav

  Non exhaustive list of features :
  - all commons POSIX file operations : open/read/write/close, opendir, readdir, mkdir
  - SSL client side credentials
  - Third party copy file
  - session reuse
*/

struct Davix_fd;
struct Davix_dir_handle;

typedef void* davix_sess_t;
typedef struct Davix_dir_handle DAVIX_DIR;
typedef struct Davix_fd DAVIX_FD;
typedef struct davix_file_desc_s* davix_file_desc_t;
typedef struct davix_auth_st* davix_auth_t;
typedef struct davix_request_params* davix_params_t;

///
/// Davix request status list
///
#define DAVIX_STATUS_OK                                     0x00
#define DAVIX_STATUS_PARTIAL_DONE                           0x01
#define DAVIX_STATUS_WEBDAV_PROPERTIES_PARSING_ERROR        0x02
#define DAVIX_STATUS_URI_PARSING_ERROR                      0x03
#define DAVIX_STATUS_SESSION_CREATION_ERROR                 0x04
#define DAVIX_STATUS_NAME_RESOLUTION_FAILURE                0x05
#define DAVIX_STATUS_CONNEXION_PROBLEM                      0x06
#define DAVIX_STATUS_REDIRECTION_NEEDED                     0x07
#define DAVIX_STATUS_CONNEXION_TIMEOUT                      0x08
#define DAVIX_STATUS_OPERATION_TIMEOUT                      0x09
#define DAVIX_STATUS_OPERATION_NOT_SUPPORTED                0x0A
#define DAVIX_STATUS_IS_NOT_A_DIRECTORY                     0x0B
#define DAVIX_STATUS_UNKNOW_ERROR                           0x0C



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



///
/// old error report system
///
#define DAVIX_ERROR_ENOTSUPPORT EPROTONOSUPPORT /*< functionality not supported */
#define DAVIX_ERROR_NOPASSWD 1000                /*< password required but not provided in the authentification callback */
#define DAVIX_ERROR_BADPASSWD 1001                /*< Bad login/password, failure */



//
// davix preproc facilities
//
#undef DAVIX_C_DECL_BEGIN
#undef DAVIX_C_DECL_END
#ifdef __cplusplus
#define DAVIX_C_DECL_BEGIN \
        extern "C" {
#define DAVIX_C_DECL_END }
#else
#define DAVIX_C_DECL_BEGIN  // void
#define DAVIX_C_DECL_END    // void
#endif

#endif // DAVIX_TYPES_H
