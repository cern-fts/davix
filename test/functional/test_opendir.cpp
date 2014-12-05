#include "test_opendir.h"

#include <davix.hpp>
#include <cstring>
#include <memory>


#include "davix_test_lib.h"


using namespace Davix;


#define MY_BUFFER_SIZE 65000

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);


    DavixError* tmp_err=NULL;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());
    if(argc > 2){
        configure_grid_env(argv[2], p);
    }



    DAVIX_DIR* d = pos.opendir(&p, argv[1], &tmp_err);

    if(d != NULL){
        struct dirent * dir = NULL;
        int n =0;
        do{
            dir= pos.readdir(d, &tmp_err);
            if(dir)
                std::cout << "N° " << ++n <<" file : " << dir->d_name << std::endl;
        }while(dir!= NULL);

        pos.closedir(d, NULL);
    }
    if(tmp_err){
        std::cout << " listing directory error "<< tmp_err->getErrMsg() << std::endl;
        return -1;
    }
    return 0;
}

