#include "test_propfind.h"


#include <core.h>
#include <curl/curlsessionfactory.h>

using namespace Davix;

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }


    try{
        std::auto_ptr<AbstractSessionFactory> s( new CURLSessionFactory());

        std::auto_ptr<HttpRequest> r (static_cast<HttpRequest*>(s->take_request(HTTP,argv[1])));

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
