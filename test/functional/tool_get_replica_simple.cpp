
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
    davix_set_log_level(0xff);

    RequestParams  params;
    DavixError* tmp_err=NULL;
    Uri url(argv[1]);
    char * cert_path = argv[2];

    if(argc > 2){
        configure_grid_env(cert_path, params);
    }

    Context c;
    std::vector<File> reps;
    File f(c, url);

    // delete unexisting dir, should fail
   reps = f.getReplicas(&params,  &tmp_err);
   if(tmp_err != NULL){
       std::cerr << "Error: " << tmp_err->getErrMsg() << std::endl;
       return -1;
   }
   if(reps.size() == 0){
       std::cout << "No Replicas" << std::endl;
       return -1;
   }

   for(std::vector<File>::iterator it = reps.begin(); it != reps.end(); ++it){
       std::cout << "Replica " << it->getUri() << std::endl;
   }
    return 0;
}

