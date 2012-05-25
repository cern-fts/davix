#include "test_propfind.h"


#include <core.h>
#include <davix_stat.h>
#include <http_backend.h>

using namespace Davix;


Auth_code mycred_auth_callback(Auth_type t, char * data,  void * userdata, GError ** err){
    if(t == DAVIX_PROXY_FULL_PEM){
        g_strlcpy(data, (char*) userdata, DAVIX_BUFFER_SIZE);
        return DAVIX_AUTH_SUCCESS;
    }
    return DAVIX_AUTH_SKIP;
}




int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);
    try{
        std::auto_ptr<AbstractSessionFactory> s( new NEONSessionFactory());
        if(argc >2 ){ // setup ops if credential is found
            s->set_ssl_ca_check(false);            // disable ssl ca check
            s->set_authentification_controller(argv[2], &mycred_auth_callback);
        }
        std::auto_ptr<HttpRequest> r (static_cast<HttpRequest*>(s->take_request(HTTP,argv[1])));
        r->disable_ssl_ca_check();

        std::vector<char> v = req_webdav_propfind(r.get());
        v.push_back('\0');
        std::cout << "content "<< (char*) &(v.at(0)) << std::endl;
    }catch(Glib::Error & e){
        std::cout << " error occures : N°" << e.code() << "  " << e.what() << std::endl;
        return -1;
    }catch(std::exception & e){
        std::cout << " error occures :" << e.what() << std::endl;
        return -1;
    }
    return 0;
}
