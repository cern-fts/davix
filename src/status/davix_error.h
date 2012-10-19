#ifndef DAVIX_ERROR_H
#define DAVIX_ERROR_H

#include <davix_types.h>

/**
  @file davix_error.h
  @author Devresse Adrien

  @brief C Error report system of davix
*/

DAVIX_C_DECL_BEGIN


///  @brief clear a davix error object and release its memory, and set the error pointer to NULL
///
void davix_error_clear(davix_error_t* ptr_err);

/// @brief create a new davix error object
///
void davix_error_setup(davix_error_t* ptr_err, const char* scope, int status_code, const char* msg);


/// @ transfer the davix error oldErr to the newErr
/// if newErr is NULL, the error is dopped
///

void davix_error_propagate(davix_error_t* newErr, davix_error_t oldErr );

const char* davix_error_msg(davix_error_t err);

int davix_error_code(davix_error_t err);

const char* davix_error_scope(davix_error_t err);


DAVIX_C_DECL_END

#endif // DAVIX_ERROR_H
