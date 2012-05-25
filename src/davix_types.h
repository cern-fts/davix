#ifndef DAVIX_TYPES_H
#define DAVIX_TYPES_H


/**
  Common Error code associated with code field in Glib::Error or GError
*/
#define DAVIX_ERROR_ENOTSUPPORT EPROTONOSUPPORT /*< functionality not supported */
#define DAVIX_ERROR_NOPASSWD 1000                /*< password required but not provided*/
#define DAVIX_ERROR_BADPASSWD 1001                /*< Bad password, failure */

typedef void* davix_sess_t;
typedef void DAVIX_DIR;
typedef void* davix_auth_t;

typedef struct{
    char * dnames;
} davix_auth_info_t;

/**
  This callback is called when a authentification is needed
  It allows to call the authentification function with the given token
  @return this callback should return 0 on success, -1 and err is set if major problem occures
*/
typedef int (*davix_auth_callback)(davix_auth_t token, const davix_auth_info_t* t, void* userdata, GError** err);




#endif // DAVIX_TYPES_H
