#pragma once
#ifndef DAVIX_DAVPOSIX_H
#define DAVIX_DAVPOSIX_H

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>



DAVIX_C_DECL_BEGIN

//
// POSIX like API
// Need Webdav support
//


/**
  @brief execute a POSIX stat query on a given WebDav URI

  POSIX-like operation,

  POSIX stat request on a given webdav endpoint
  @param sess : davix session handle
  @param params : request parameters, OPTIONAL
  @param url: url of the webdav point
  @param st : stat structure
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_posix_stat(davix_sess_t sess, davix_params_t params, const char* url, struct stat * st, GError** err);



/**
  @brief execute a POSIX mkdir query on a given WebDav URI

  POSIX-like operation,

  create a directory ( collection)  at the given url


  @param sess : davix session handle
  @param params : request parameters, OPTIONAL
  @param url: url of the folder to create
  @param right: remote file right
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_posix_mkdir(davix_sess_t sess, davix_params_t _params, const char* url,  mode_t right, GError** err);



/**
  @brief execute a POSIX opendir query on a given WebDav URI

  POSIX-like operation

  open a directory for a listing operation

  @param sess : davix session handle
  @param params : request parameters, OPTIONAL
  @param url: url of the folder to create
  @param err : GError error handling system
  @return directy stream pointer or NULL if error
*/
DAVIX_DIR* davix_posix_opendir(davix_sess_t sess, davix_params_t _params, const char* url, GError** err);



/**
  @brief execute a POSIX readdir query on a given WebDav URI

  POSIX-like operation

  returns a pointer to a dirent structure representing the next directory entry in the directory stream pointed to by dirp

  @param sess : davix session handle
  @param d: directory stream
  @param err : GError error handling system
  @return pointer to struct dirent or NULL if error
*/
struct dirent* davix_posix_readdir(davix_sess_t sess, DAVIX_DIR* d, GError** err);


/**
  @brief execute a POSIX closedir query on a given WebDav URI

  POSIX-like operation

  close an open directory stream

  @param sess : davix session handle
  @param d: directory stream
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_posix_closedir(davix_sess_t sess, DAVIX_DIR* d, GError** err);


DAVIX_C_DECL_END

#endif // DAVIX_DAVPOSIX_H
