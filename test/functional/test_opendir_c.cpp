#include "test_opendir_c.h"


#include <davix.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <logger/logger.h>


#include "davix_test_lib.h"

#define MY_BUFFER_SIZE 65000



int main(int argc, char** argv){

    davix_params_t params = NULL;
    davix_sess_t sess = NULL;
    davix_error_t tmp_err=NULL;
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);


        sess = davix_context_new(&tmp_err);
        if(!sess){
            std::cout << " handle creation error" << std::endl;
            return -2;
        }
        if(argc > 2){
            params = davix_params_new();
            davix_params_set_auth_callback(params, mycred_auth_callback, argv[2], &tmp_err);
            davix_params_set_ssl_check(params, FALSE, &tmp_err);
          //  davix_set_default_session_params(ctxt, p, NULL);
        }




        DAVIX_DIR* d = davix_posix_opendir(sess, params, argv[1], &tmp_err);
        if(!d){
            std::cout << "davix_opendir_error" << davix_error_msg(tmp_err) << std::endl;
            return -1;
        }

        struct dirent * dir = NULL;
        do{
            dir= davix_posix_readdir(sess,d, &tmp_err);
            if(dir)
                std::cout << "N° " << dir->d_off <<" file : " << dir->d_name << std::endl;
        }while(dir!= NULL);

        davix_posix_closedir(sess,d, &tmp_err);
    return 0;
}

