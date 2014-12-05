#include "test_propfind.h"


#include <davix.hpp>
#include <fileops/davmeta.hpp>
#include <memory>
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

    Context c;
    if(argc >2 ){ // setup ops if credential is found
         params.setSSLCAcheck(false);
    }
    HttpRequest r(c, argv[1], &tmp_err);
    r.setParameters(params);
    r.addHeaderField("Depth", "1");

    std::vector<char> body = req_webdav_propfind(&r, &tmp_err);
    std::string v(body.begin(), body.end());

    std::cout << "content "<< v << std::endl;
    if(tmp_err){
        std::cerr << " req error " << tmp_err->getErrMsg() << std::endl;
    }
    return 0;
}
