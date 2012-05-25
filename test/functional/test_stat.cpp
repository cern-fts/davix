#include "test_stat.h"

#include <core.h>
#include <curl/curlsessionfactory.h>
#include <glibmm/init.h>

using namespace Davix;


 Auth_code mycred_auth_callback(Auth_type t, char * data,  void * userdata, GError ** err){
     if(t == DAVIX_FULL_PEM){
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

    char cert_path[2048];
    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);

    try{
        Glib::RefPtr<Core> c= Core::create(new CURLSessionFactory());
        if(argc > 2){
            configure_grid_env(argv[2], c);
        }


        Glib::RefPtr<Stat> stat_ops = c->getStat();
        struct stat st;
        stat_ops->stat(argv[1], &st);

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

