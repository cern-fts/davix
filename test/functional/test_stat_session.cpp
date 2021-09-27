#include "test_stat_session.h"

#include <davix.hpp>
#include <cstring>
#include <memory>

#include "davix_test_lib.h"

using namespace Davix;

int n_call=0;

int main(int argc, char** argv){
    if( argc < 3){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [CERTIFICATE_PATH] [url] [url] ....  " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);


    DavixError* tmp_err=NULL;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());
    if(argc > 2){
        configure_grid_env(argv[1], p);
    }


    for(int i =2 ; i< argc; ++i){
        struct stat st;
        if( pos.stat(&p,argv[i], &st, &tmp_err) ==0){
            std::cout << "stat success" << std::endl;
            std::cout << " atime : " << st.st_atime << std::endl;
            std::cout << " mtime : " << st.st_mtime << std::endl;
            std::cout << " ctime : " << st.st_ctime << std::endl;
            std::cout << " mode : 0" << std::oct << st.st_mode << std::endl;
            std::cout << " len : " << st.st_size << std::endl;
        }else{
            std::cerr << " davix_stat error " << tmp_err->getErrMsg();
            return -1;
        }



    }



    std::cout << "authentication callback has been called " << n_call << std::endl;

    return 0;
}

