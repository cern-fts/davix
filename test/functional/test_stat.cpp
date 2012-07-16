#include "test_stat.h"

#include <core.hpp>
#include <http_backend.hpp>
#include <glibmm/init.h>

using namespace Davix;



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
        *((char*) mempcpy(login, auth_string, p-auth_string)) = '\0';
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


static void configure_grid_env(char * cert_path, Core  * core){
    AbstractSessionFactory* f = core->getSessionFactory();
    RequestParams params;
    params.set_ssl_ca_check(false);
    params.set_authentification_controller(cert_path, &mycred_auth_callback);
    f->set_parameters(params);
}

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);

    try{
        std::auto_ptr<Core> c( new Core(new NEONSessionFactory()));
        if(argc > 2){
            configure_grid_env(argv[2], c.get());
        }


        struct stat st;
        c->stat(argv[1], &st);

        std::cout << "stat success" << std::endl;
        std::cout << " atime : " << st.st_atime << std::endl;
        std::cout << " mtime : " << st.st_mtime << std::endl;
        std::cout << " ctime : " << st.st_ctime << std::endl;
        std::cout << " mode : 0" << std::oct << st.st_mode << std::endl;
        std::cout << " len : " << st.st_size << std::endl;
    }catch(Glib::Error & e){
        std::cout << " error occures : N°" << e.code() << "  " << e.what() << std::endl;
        return -1;
    }catch(std::exception & e){
        std::cout << " error occures :" << e.what() << std::endl;
        return -1;
    }
    return 0;
}

