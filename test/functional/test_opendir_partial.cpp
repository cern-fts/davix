#include "test_opendir_partial.h"


#include <davixcontext.hpp>
#include <memory>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <file/davposix.hpp>

#include "davix_test_lib.h"

/**
  Execute an incomplete readdir of a directory, and verify the correct closedir with the memory cleaning
*/

using namespace Davix;


#define MY_BUFFER_SIZE 65000




int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
       std::cout <<"\t" << argv[0] << " [url] [size_to_read]  " << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [size_to_read] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);
    int max_read= atoi(argv[2]);
    std::cout << " end of the dir number " << max_read << std::endl;

    DavixError* tmp_err=NULL;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());

    if(argc > 3){
        configure_grid_env(argv[3], p);
    }



    DAVIX_DIR* d = pos.opendir(&p, argv[1], &tmp_err);

    if(d){
        struct dirent * dir = NULL;
        int n= 0;
        do{
            dir= pos.readdir(d, &tmp_err);
            if(dir)
                std::cout << "N° " << n <<" file : " << dir->d_name << std::endl;
            n++;
        }while(dir!= NULL && max_read > n);
        if(dir == NULL){
            std::cout << " Normal end of the directory, too little directory, error in the test " << std::endl;
            return -3;
     }
   }

    pos.closedir(d, NULL);
    if(tmp_err){
        std::cout << " listing directory error "<< tmp_err->getErrMsg() << std::endl;
        return -1;
    }
    return 0;
}

