#include "test_stat.h"

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
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
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


    struct stat st;
    if( pos.stat(&p, argv[1], &st, &tmp_err) < 0){
        std::cerr << " error on stat req " << tmp_err->getErrMsg() << std::endl;
        return -1;
    }

    std::cout << "stat success" << std::endl;
    std::cout << " atime : " << st.st_atime << std::endl;
    std::cout << " mtime : " << st.st_mtime << std::endl;
    std::cout << " ctime : " << st.st_ctime << std::endl;
    std::cout << " mode : 0" << std::oct << st.st_mode << std::endl;
    std::cout << " len : " << st.st_size << std::endl;

    return 0;
}

