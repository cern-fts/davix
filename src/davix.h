#ifndef DAVIX_H
#define DAVIX_H

#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

///
///  @file davix.h
///  @author Devresse Adrien
///
///  @brief C header file of Davix
///  Davix is a high level HTTP/Webdav library
///  for file management and file access.
///

#ifndef __DAVIX_INSIDE__
#define __DAVIX_INSIDE__
#endif

// logger
#include <logger/davix_logger.h>

// general types
#include <davix_types.h>

// parameters
#include <params/davixrequestparams.h>

// error code and status
#include <status/davix_error.h>

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








DAVIX_C_DECL_END

#endif // DAVIX_H
