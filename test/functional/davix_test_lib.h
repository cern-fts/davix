#ifndef DAVIX_TEST_LIB_H
#define DAVIX_TEST_LIB_H

#include <davix.hpp>
#include <cassert>

int mycred_auth_callback_x509(void* userdata, const Davix::SessionInfo & info, Davix::X509Credential * cert, Davix::DavixError** err);

void configure_grid_env(char * auth_args, Davix::RequestParams&  p);

void configure_grid_env_bis(char * auth_args, Davix::RequestParams&  p);


char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff);

Davix::Uri generate_random_uri(const Davix::Uri & url_directory, const std::string & prefix);

char * generate_random_string_content(size_t size);


#define DAV_ASSERT_TRUE(test, failure_msg) \
    do{ \
    if(! (test) ){ \
        std::cerr << "f:"<< __FILE__ << ":" << __LINE__ << ": " << failure_msg << std::endl; \
        exit(-1); \
    } \
    }while(0)


#endif // DAVIX_TEST_LIB_H
