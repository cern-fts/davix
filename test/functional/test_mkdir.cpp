
#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davposix.hpp>

#include <string>
#include <sstream>
#include <cmath>
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

    srand(time(NULL));
    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);


    RequestParams  p;
    DavixError* tmp_err=NULL;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    std::ostringstream oss;
    oss << argv[1] << "/"<< (rand()%20000);
    std::string a = oss.str();

    int ret = pos.mkdir(&p, a.c_str(), 0777, &tmp_err);


    if(ret ==0)
        std::cout << "mkdir  success !" << std::endl;
    else
        std::cout << "mkdir error "<< tmp_err->getErrMsg() << std::endl;

    return 0;
}

