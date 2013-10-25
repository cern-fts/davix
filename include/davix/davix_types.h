#ifndef DAVIX_TYPES_H
#define DAVIX_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

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

struct Davix_context;
struct Davix_error;

// alias
typedef struct Davix_context* davix_sess_t;
typedef struct Davix_error* davix_error_t;
typedef struct davix_file_desc_s* davix_file_desc_t;
typedef struct davix_auth_st* davix_auth_t;
typedef struct davix_request_params* davix_params_t;

// Davix Large File Support
#if  ( __WORDSIZE == 32 ) || \
        ( SIZE_MAX ==  (4294967295U) ) || \
        ( defined __WIN32 )

typedef uint64_t dav_off_t;
typedef uint64_t dav_size_t;
typedef int64_t dav_ssize_t;
#else
typedef off_t dav_off_t;
typedef size_t dav_size_t;
typedef ssize_t dav_ssize_t;
#endif

// block size
#define DAVIX_BLOCK_SIZE 4096
#define DAVIX_MAX_BLOCK_SIZE 16777216

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
