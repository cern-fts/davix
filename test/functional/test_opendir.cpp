#include "test_opendir.h"


#include <core.h>
#include <http_backend.h>
#include <glibmm/init.h>

using namespace Davix;


#define MY_BUFFER_SIZE 65000

 Auth_code mycred_auth_callback(Auth_type t, char * data,  void * userdata, GError ** err){
     if(t == DAVIX_PROXY_FULL_PEM){
         g_strlcpy(data, (char*) userdata, DAVIX_BUFFER_SIZE);
         return DAVIX_AUTH_SUCCESS;
     }
     return DAVIX_AUTH_SKIP;
 }

static void configure_grid_env(char * cert_path, const Glib::RefPtr<Core>  & core){
    AbstractSessionFactory* f = core->getSessionFactory();
    f->set_ssl_ca_check(false);            // disable ssl ca check
    f->set_authentification_controller(cert_path, &mycred_auth_callback);
}

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_WARNING);

    try{
        Glib::RefPtr<Core> c= Core::create(new NEONSessionFactory());
        c->set_buffer_size(MY_BUFFER_SIZE);
        if(argc > 2){
            configure_grid_env(argv[2], c);
        }



        DAVIX_DIR* d = c->opendir(argv[1]);

        struct dirent * dir = NULL;
        do{
            dir= c->readdir(d);
            if(dir)
                std::cout << "N° " << dir->d_off <<" file : " << dir->d_name << std::endl;
        }while(dir!= NULL);

        c->closedir(d);
    }catch(Glib::Error & e){
        std::cout << " error occures : N°" << e.code() << "  " << e.what() << std::endl;
        return -1;
    }catch(std::exception & e){
        std::cout << " error occures :" << e.what() << std::endl;
        return -1;
    }
    return 0;
}

