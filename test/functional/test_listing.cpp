#include "test_listing.hpp"

#include <davix.hpp>


#include "davix_test_lib.h"


using namespace Davix;


#define MY_BUFFER_SIZE 65000

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    RequestParams  params;
    char * cert_path = argv[2];

    if(argc > 2){
        configure_grid_env(cert_path, params);
    }

    Uri u(argv[1]);

    try{

        Context c;
        File f(c, u);
        int i =0;
        File::Iterator it = f.listCollection(&params);

        do{
            i +=1;
            std::cout << " " << it.name() << " stat " << it.info().mode << " " << it.info().size << std::endl;
        }while(it.next());

    }catch(DavixException & e){
        std::cerr << "Error: (" <<e.scope() << ") "  << e.what() << std::endl;
        return -1;
    }

    return 0;
}

