#ifndef DAVIX_TEST_LIB_H
#define DAVIX_TEST_LIB_H

#include <davix.hpp>

int mycred_auth_callback_x509(void* userdata, const Davix::SessionInfo & info, Davix::X509Credential * cert, Davix::DavixError** err);

DAVIX_C_DECL_BEGIN


char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff);

char * generate_random_string_content(size_t size);


DAVIX_C_DECL_END

#endif // DAVIX_TEST_LIB_H
