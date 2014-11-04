
#include <davix.hpp>
#include <memory>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "davix_test_lib.h"


using namespace Davix;

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    srand(time(NULL));
    davix_set_log_level(DAVIX_LOG_ALL);


    RequestParams  p;
    DavixError* tmp_err=NULL;
    //std::auto_ptr<Context> c( new Context());
    Context c;

    DavPosix pos(&c);

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    std::string url = argv[1];
    std::string a =  generate_random_uri(Davix::Uri(url), "test_move").getString();
    std::string b = a+"cake";

    int ret = 0;

    // create dir
    if( (ret = pos.mkdir(&p, a.c_str(), 0777, &tmp_err) <0))
    {
        std::cerr << "mkdir error "<< tmp_err->getErrMsg() << std::endl;
        return -1;
    }
    // rename file
    if( (ret = pos.rename(&p, a.c_str(), b.c_str(), &tmp_err) <0))
    {
        std::cerr << "mv error "<< tmp_err->getErrMsg() << std::endl;
        return -1;
    }
    // remove dir
    DavFile f(c, b);
    f.deletion(&p, &tmp_err);

    return 0;
}
