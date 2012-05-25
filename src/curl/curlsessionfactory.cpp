#include "curlsessionfactory.h"
#include "curlrequest.h"


#include <glibmm/error.h>

namespace Davix {

CURLSessionFactory::CURLSessionFactory()
{
     curl_global_init(CURL_GLOBAL_ALL ); // global init of curl
}

CURLSessionFactory::~CURLSessionFactory(){
    curl_global_cleanup();
}

Request* CURLSessionFactory::take_request(RequestType typ, const std::string & url){
    CURL * c = curl_easy_init();
    if(c == NULL)
        throw Glib::Error(Glib::Quark("CURLSessionFactory::take_request"), EINVAL, "invalid CURL init");
    CURLRequest* req =  new CURLRequest(c, typ, url);
    return static_cast<Request*>(req);
}


void CURLSessionFactory::release_request(Request *req){
    delete req;
}

}
