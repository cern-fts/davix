#include "test_propfind.h"


#include <core.hpp>
#include <davix_stat.hpp>
#include <http_backend.hpp>

using namespace Davix;


int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, GError** err){
    GError * tmp_err=NULL;
    int ret = davix_set_pkcs12_auth(token, (const char*)userdata, (const char*)NULL, &tmp_err);
    if(ret != 0){
        fprintf(stderr, " FATAL authentification Error : %s", tmp_err->message);
        g_propagate_error(err, tmp_err);
    }

    return ret;
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
