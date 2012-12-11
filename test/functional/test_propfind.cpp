#include "test_propfind.h"


#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davix_stat.hpp>

#include "davix_test_lib.h"

using namespace Davix;




int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);
    RequestParams params;
    DavixError* tmp_err=NULL;

    std::auto_ptr<AbstractSessionFactory> s( new NEONSessionFactory());
    if(argc >2 ){ // setup ops if credential is found
         params.setSSLCAcheck(false);
    }
    std::auto_ptr<HttpRequest> r (static_cast<HttpRequest*>(s->create_request(argv[1], &tmp_err)));
    if(r.get() != NULL){
        r->setParameters(params);
        r->addHeaderField("Depth", "1");

        std::vector<char> v = req_webdav_propfind(r.get(), &tmp_err);
        v.push_back('\0');
        std::cout << "content "<< (char*) &(v.at(0)) << std::endl;
    }
    if(tmp_err){
        std::cerr << " req error " << tmp_err->getErrMsg() << std::endl;
    }
    return 0;
}
