#ifndef DAVIX_TEST_LIB_H
#define DAVIX_TEST_LIB_H

#include <davix.hpp>
#include <cassert>
#include "davix_test_lib_c.h"

int mycred_auth_callback_x509(void* userdata, const Davix::SessionInfo & info, Davix::X509Credential * cert, Davix::DavixError** err);

void configure_grid_env(char * auth_args, Davix::RequestParams&  p);

void configure_grid_env_bis(char * auth_args, Davix::RequestParams&  p);


char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff);

char * generate_random_string_content(size_t size);

#endif // DAVIX_TEST_LIB_H
