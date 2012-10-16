#include "test_opendir_c.h"


#include <davix.h>
#include <string.h>
#include <cstdio>
#include <glibmm.h>
#include <iostream>
#include <logging/logger.h>

#define MY_BUFFER_SIZE 65000

int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, GError** err){
    GError * tmp_err=NULL;
    char login[2048];
    char passwd[2048];
    char *p,*auth_string =(char*) userdata;
    int ret ;
    bool login_password_auth_type = false;
    memset(login,'\0', sizeof(char)*2048);

    if( (p = strchr(auth_string,':')) != NULL)
        login_password_auth_type = true;

    if(login_password_auth_type ){
        *((char*)mempcpy(login, auth_string, p-auth_string)) ='\0';
        strcpy(passwd, p+1 );
        ret = davix_set_login_passwd_auth(token, login, passwd, &tmp_err);

    }else{
        ret = davix_set_pkcs12_auth(token, (const char*)userdata, (const char*)NULL, &tmp_err);
    }

    if(ret != 0){
        fprintf(stderr, " FATAL authentification Error : %s", tmp_err->message);
        g_propagate_error(err, tmp_err);
    }
    return ret;
}



int main(int argc, char** argv){

    davix_params_t params = NULL;
    davix_sess_t sess = NULL;
    GError* tmp_err=NULL;
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);

    try{
        sess = davix_context_new(&tmp_err);
        if(argc > 2){
            params = davix_params_new();
            davix_params_set_auth_callback(params, mycred_auth_callback, argv[2], &tmp_err);
            davix_params_set_ssl_check(params, FALSE, &tmp_err);
          //  davix_set_default_session_params(ctxt, p, NULL);
        }




        DAVIX_DIR* d = davix_posix_opendir(sess, params, argv[1], &tmp_err);
        if(tmp_err){
            std::cout << "davix_opendir_error" << tmp_err->message << std::endl;
            return -1;
        }

        struct dirent * dir = NULL;
        do{
            dir= davix_posix_readdir(sess,d, &tmp_err);
            if(dir)
                std::cout << "N° " << dir->d_off <<" file : " << dir->d_name << std::endl;
        }while(dir!= NULL);

        davix_posix_closedir(sess,d, &tmp_err);
    }catch(Glib::Error & e){
        std::cout << " error occures : N°" << e.code() << "  " << e.what() << std::endl;
        return -1;
    }catch(std::exception & e){
        std::cout << " error occures :" << e.what() << std::endl;
        return -1;
    }
    return 0;
}

