#pragma once
#ifndef DAVIX_POSIX_H
#define DAVIX_POSIX_H

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <davix_types.h>



/**
  @file davix_posix.h
  @author Devresse Adrien

  @brief C POSIX like API of davix
 */


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
  @param params : request parameters, can be NULL
  @param url : url of the webdav file/directory
  @param st : stat structure
  @param err : GError error handling system
  @return 0 if success else a negative value
*/
int davix_posix_stat(davix_sess_t sess, davix_params_t params, const char* url, struct stat * st, GError** err);



/**
  @brief execute a POSIX mkdir query on a given WebDav URI

  POSIX-like operation,

  create a directory ( collection)  at the given url


  @param sess : davix session handle
  @param params : request parameters, can be NULL
  @param url : url of the folder to create
  @param right : remote file right
  @param err : GError error handling system
  @return 0 if success else a negative value
*/
int davix_posix_mkdir(davix_sess_t sess, davix_params_t _params, const char* url,  mode_t right, GError** err);



/**
  @brief execute a POSIX opendir query on a given WebDav URI

  POSIX-like operation

  open a directory for a listing operation

  @param sess : davix session handle
  @param params : request parameters, can be NULL
  @param url : url of the folder to list
  @param err : GError error handling system
  @return directory stream pointer or NULL if error
*/
DAVIX_DIR* davix_posix_opendir(davix_sess_t sess, davix_params_t params, const char* url, GError** err);



/**
  @brief execute a POSIX readdir query on a given WebDav URI

  POSIX-like operation

  returns a pointer to a dirent structure representing the next directory entry in the directory stream pointed to by dirp

  @param sess : davix session handle
  @param dirp : directory stream
  @param err : GError error handling system
  @return pointer to a dirent structure or NULL if error
*/
struct dirent* davix_posix_readdir(davix_sess_t sess, DAVIX_DIR* dirp, GError** err);


/**
  @brief execute a POSIX closedir query on a given WebDav URI

  POSIX-like operation

  close an open directory stream

  @param sess : davix session handle
  @param d : directory stream
  @param err : GError error handling system
  @return 0 if success else -1
*/
int davix_posix_closedir(davix_sess_t sess, DAVIX_DIR* dirp, GError** err);



/**
  @brief open  a file in a POSIX-like approach with HTTP(S)

  open a file for reading or/and writing

  @param sess : davix session handle
  @param params : request parameters, can be NULL
  @param url : url of the file to open
  @param err : GError error handling system
  @return directory stream pointer or NULL if error
*/
DAVIX_FD* davix_posix_open(davix_sess_t sess, davix_params_t params, const char* url, int flags, GError** err);


/**
  @brief read a file in a POSIX-like approach with HTTP(S)
  behavior similar to the POSIX read function

  @param sess : davix session handle
  @param fd : davix file descriptor
  @param buf : buffer to fill
  @param count : maximum number of bytes to read
  @param err: GError error handling system
  @return the size of data or a negative value if an error occured
 */
ssize_t davix_posix_read(davix_sess_t sess, DAVIX_FD* fd, void* buffer, size_t read_size, GError* err);


/**
  @brief write a file in a POSIX-like approach with HTTP(S)
  behavior similar to the POSIX write function

  @param sess : davix session handle
  @param fd : davix file descriptor
  @param buf : buffer with the write content
  @param count : number of bytes to write
  @return the size of the written data or a negative value if an error occured
 */
ssize_t davix_posix_write(davix_sess_t sess, DAVIX_FD* fd, const void* buffer, size_t write_size, GError* err);


/**
  @brief move the cursor a davix file with HTTP(S)
  behavior similar to the POSIX lseek function

  @param sess : davix session handle
  @param fd : davix file descriptor
  @param offset : offset in byte inside the file
  @param flags : lseek flags, similar to the lseek function
  @return the offset position or a negative value if an error occures
 */
off_t davix_posix_lseek(davix_sess_t sess, DAVIX_FD* fd, off_t offset, int flags, GError* err);


int davix_posix_close(davix_sess_t sess, DAVIX_FD* fd, GError** err);

DAVIX_C_DECL_END

#endif // DAVIX_POSIX_H
