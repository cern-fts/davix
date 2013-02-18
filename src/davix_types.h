#ifndef DAVIX_TYPES_H
#define DAVIX_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/**
  @file davix_types.h
  @author Devresse Adrien


  @brief davix types declarations

  Davix is a file access and file management library on top of HTTP/Webdav

*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

#ifndef DAVIX_EXPORT
#define DAVIX_EXPORT
#endif

struct Davix_fd;
struct Davix_dir_handle;
struct Davix_context;
struct Davix_error;

// C++ -> C alias
typedef struct Davix_context* davix_sess_t;
typedef struct Davix_error* davix_error_t;
typedef struct Davix_dir_handle DAVIX_DIR;
typedef struct Davix_fd DAVIX_FD;
typedef struct davix_file_desc_s* davix_file_desc_t;
typedef struct davix_auth_st* davix_auth_t;
typedef struct davix_request_params* davix_params_t;

// Davix Large File Support
typedef uint64_t dav_off_t;
typedef uint64_t dav_size_t;
typedef int64_t dav_ssize_t;



/// Davix status codes
typedef enum davix_status_e {
    DAVIX_STATUS_OK  = 0x00,
    DAVIX_STATUS_PARTIAL_DONE,
    DAVIX_STATUS_WEBDAV_PROPERTIES_PARSING_ERROR,
    DAVIX_STATUS_URI_PARSING_ERROR,
    DAVIX_STATUS_SESSION_CREATION_ERROR,
    DAVIX_STATUS_NAME_RESOLUTION_FAILURE,
    DAVIX_STATUS_CONNECTION_PROBLEM,
    DAVIX_STATUS_REDIRECTION_NEEDED,
    DAVIX_STATUS_CONNECTION_TIMEOUT,
    DAVIX_STATUS_OPERATION_TIMEOUT,
    DAVIX_STATUS_OPERATION_NOT_SUPPORTED,
    DAVIX_STATUS_IS_NOT_A_DIRECTORY,
    DAVIX_STATUS_UNKNOW_ERROR,
    DAVIX_STATUS_INVALID_FILE_HANDLE,
    DAVIX_STATUS_ALREADY_RUNNING,
    DAVIX_STATUS_AUTHENTIFICATION_ERROR,
    DAVIX_STATUS_LOGIN_PASSWORD_ERROR,
    DAVIX_STATUS_CREDENTIAL_NOT_FOUND,
    DAVIX_STATUS_PERMISSION_REFUSED,
    DAVIX_STATUS_FILE_NOT_FOUND,
    DAVIX_STATUS_IS_A_DIRECTORY,
    DAVIX_STATUS_SYSTEM_ERROR,
    DAVIX_STATUS_FILE_EXIST,
    DAVIX_STATUS_INVALID_ARG
} davix_status_t;


//// protocol type
typedef enum davix_request_protocol_e{
    DAVIX_PROTOCOL_WEBDAV=0,
    DAVIX_PROTOCOL_HTTP
} davix_request_protocol_t;





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
