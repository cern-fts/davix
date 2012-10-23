#include "test_opendir.h"


#include <davixcontext.hpp>
#include <string.h>
#include <http_backend.hpp>
#include <glibmm/init.h>
#include <posix/davposix.hpp>

using namespace Davix;


#define MY_BUFFER_SIZE 65000

int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, davix_error_t* err){
    davix_error_t tmp_err=NULL;
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
        fprintf(stderr, " FATAL authentification Error : %s", davix_error_msg(tmp_err));
        davix_error_propagate(err, tmp_err);
    }
    return ret;
}



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



    DAVIX_DIR* d = pos.opendir(&p, argv[1], &tmp_err);

    if(d != NULL){
        struct dirent * dir = NULL;
        do{
            dir= pos.readdir(d, &tmp_err);
            if(dir)
                std::cout << "N° " << dir->d_off <<" file : " << dir->d_name << std::endl;
        }while(dir!= NULL);

        pos.closedir(d, NULL);
    }
    if(tmp_err){
        std::cout << " listing directory error "<< tmp_err->getErrMsg() << std::endl;
        return -1;
    }
    return 0;
}

