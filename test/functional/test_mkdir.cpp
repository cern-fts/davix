#include "test_stat.h"

#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davposix.hpp>

#include <string>
#include <sstream>
#include <cmath>
#include <string.h>


using namespace Davix;


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
        *((char*) mempcpy(login, auth_string, p-auth_string)) = '\0';
        strcpy(passwd, p+1 );
        ret = davix_auth_set_login_passwd(token, login, passwd, &tmp_err);

    }else{
        ret = davix_auth_set_pkcs12_cli_cert(token, (const char*)userdata, (const char*)NULL, &tmp_err);
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

