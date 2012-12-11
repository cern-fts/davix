#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davposix.hpp>
#include <string.h>

#include "davix_test_lib.h"

using namespace Davix;



static void configure_grid_env(char * cert_path, RequestParams&  p){
    p.setSSLCAcheck(false);
    p.setAuthentificationCallback(cert_path, &mycred_auth_callback);
}

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [base_dir]" << std::endl;
        std::cout <<"\t" << argv[0] << " [base_dir] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);

    DavixError* tmp_err=NULL;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    char buff[2048];

    generate_random_uri(argv[1], "davix_test_stat", buff, 2048);

    struct stat st;
    // do a first stat, should return enoent
    std::cout << " stat enoent dir " << buff  << std::endl;
    int ret = pos.stat(&p, buff, &st, &tmp_err);
    g_assert( ret < 0);
    g_assert(tmp_err && StatusCode::FileNotFound == tmp_err->getStatus());
    DavixError::clearError(&tmp_err);

    std::cout << " create dir " << buff  << std::endl;
    ret = pos.mkdir(&p, buff, 0755, &tmp_err);
    g_assert(0 == ret);
    g_assert(NULL == tmp_err);

    std::cout << " stat new dir " << std::endl;
    ret = pos.stat(&p, buff, &st, &tmp_err);
    g_assert(0 == ret);
    g_assert(S_ISDIR(st.st_mode));

    std::cout << "stat success" << std::endl;
    std::cout << " atime : " << st.st_atime << std::endl;
    std::cout << " mtime : " << st.st_mtime << std::endl;
    std::cout << " ctime : " << st.st_ctime << std::endl;
    std::cout << " mode : 0" << std::oct << st.st_mode << std::endl;
    std::cout << " len : " << st.st_size << std::endl;

    return 0;
}

