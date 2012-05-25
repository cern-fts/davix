#include "neonsessionfactory.h"

#include <glibmm/error.h>
#include <glibmm/quark.h>

#include <cstring>

namespace Davix {

const char* proto_support[] = { "http", "https", NULL };
const unsigned int ports[] = { 80 ,443 , 0};

NEONSessionFactory::NEONSessionFactory()
{
    ne_sock_init();
    _ca_check=true;
    _user_auth_callback_data = NULL;
    _call = NULL;
}

NEONSessionFactory::~NEONSessionFactory(){
    ne_sock_exit();
}

Request* NEONSessionFactory::take_request(RequestType typ, const std::string &url){
    std::string host, protocol, path;
    unsigned int port;
    parse_http_neon_url(url, protocol, host, path, &port);
    ne_session* sess = create_session(protocol, host, port);
    NEONRequest* req = new NEONRequest(sess, typ, path, _user_auth_callback_data, _call);
    return static_cast<Request*>(req);
}

void NEONSessionFactory::set_authentification_controller(void *userdata, davix_auth_callback call){
    _call = call;
    _user_auth_callback_data = userdata;
}

void NEONSessionFactory::release_request(Request *req){
    delete req;
}

void NEONSessionFactory::set_ssl_ca_check(bool chk){
    _ca_check = chk;
}


ne_session* NEONSessionFactory::create_session(const std::string & protocol, const std::string &host, unsigned int port){
    ne_session* se;
    int status;

    se = ne_session_create(protocol.c_str(), host.c_str(), (int) port);
    return se;
}

void parse_http_neon_url(const std::string &url, std::string &protocol, std::string &host, std::string &path, unsigned int * port){
    char * c_url = (char*) url.c_str();
    char * comma;
    if( (comma = strchr(c_url,':')) != NULL){ // determine protocol
        char ** proto;
        for(proto= (char**) proto_support;
            *proto != NULL;
            ++proto){
            if(strncmp(c_url, *proto, comma - c_url) == 0){
                protocol = std::string(c_url, comma-c_url);
                break;
            }
        }
        // verify if end of list -> failure
        if(*proto != NULL){
           char * second_slash, *first_slash_next = comma +1;
           while(*first_slash_next == '/') // finish prefix
               first_slash_next ++;
           if(*first_slash_next != '\0'){ // if not end, find the path
               second_slash = strchr(first_slash_next, '/');
               std::string host_port;
               if(second_slash == NULL){
                   path = std::string("/");
                   host_port = std::string(first_slash_next);
               }else{
                   path = std::string(second_slash);
                   host_port = std::string(first_slash_next, second_slash- first_slash_next);
               }
               const char * p_host = host_port.c_str();
               const char * comma_port = strchr(p_host, ':');
               *port=0;
               if(comma_port != NULL){
                   *port = (unsigned int) strtoul(comma_port+1,NULL,10);
                    if( *port != ULONG_MAX && *port != 0){
                        host = std::string(p_host, comma_port- p_host);
                        davix_log_debug(" host: %s path: %s protocol: %s port: %d", host.c_str(), path.c_str(), protocol.c_str(), *port);
                        return;
                    }
               }else{
                   *port = (*proto  == (proto_support[0]))?ports[0]:ports[1];
                   host = std::string(p_host);
                   davix_log_debug(" host: %s path: %s protocol: %s port: %d", host.c_str(), path.c_str(), protocol.c_str(), *port);
                   return;
               }

           }

        }
    }
   throw Glib::Error(Glib::Quark("NEONSessionFactory::parse_http_neon_url"), EINVAL, std::string("Invalid url format : ").append(url));
}

} // namespace Davix
