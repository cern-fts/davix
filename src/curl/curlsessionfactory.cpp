#include "curlsessionfactory.h"
#include "curlrequest.h"


#include <glibmm/error.h>

namespace Davix {

CURLSessionFactory::CURLSessionFactory()
{
     curl_global_init(CURL_GLOBAL_ALL ); // global init of curl
     _ca_check=true;
     _user_auth_callback_data = NULL;
     _call = NULL;
}

CURLSessionFactory::~CURLSessionFactory(){
    curl_global_cleanup();
}

Request* CURLSessionFactory::take_request(RequestType typ, const std::string & url){
    CURL * c = curl_easy_init();
    if(c == NULL)
        throw Glib::Error(Glib::Quark("CURLSessionFactory::take_request"), EINVAL, "invalid CURL init");
    CURLRequest* req =  new CURLRequest(c, typ, url, _call, _user_auth_callback_data);
    if(_ca_check==false) // disable ssl ca check
        req->disable_ssl_ca_check();

    return static_cast<Request*>(req);
}


void CURLSessionFactory::release_request(Request *req){
    delete req;
}


void CURLSessionFactory::set_ssl_ca_check(bool chk){
    _ca_check = false;
}

void CURLSessionFactory::set_authentification_controller(void *userdata, davix_auth_callback call){
    _user_auth_callback_data = userdata;
    _call = call;
}

}
