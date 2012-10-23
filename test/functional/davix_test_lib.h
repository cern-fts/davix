#ifndef DAVIX_TEST_LIB_H
#define DAVIX_TEST_LIB_H

#include <davix.h>

DAVIX_C_DECL_BEGIN


int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, davix_error_t* err);


char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff);

char * generate_random_string_content(size_t size);


DAVIX_C_DECL_END

#endif // DAVIX_TEST_LIB_H
