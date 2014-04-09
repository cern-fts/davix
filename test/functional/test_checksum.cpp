
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
    if( argc < 3){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [algo]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [algo] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    RequestParams  p;
    DavixError* tmp_err=NULL;

    Uri u(argv[1]);
    char* algo = argv[2];
    Context c;
    File f(c,u);

    if(argc > 3){
        configure_grid_env(argv[3], p);
    }

    std::string chk;
    int ret = f.checksum(&p, chk, algo, &tmp_err);

    if(ret ==0)
        std::cout << algo << " "<<  chk << std::endl;
    else
        std::cout << "checksum error "<< tmp_err->getErrMsg() << std::endl;

    return 0;
}

