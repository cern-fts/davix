#include "test_directory.hpp"

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
    struct stat st;
    int res =-1;
    DavixError* tmp_err=NULL;
    char * base_dir = argv[1];
    char * cert_path = argv[2];
    char buffer[2048];

    if(argc > 2){
        configure_grid_env(cert_path, params);
    }


    srand(time(NULL));
    davix_set_log_level(DAVIX_LOG_ALL);
    generate_random_uri(base_dir, "rmdir_unlink_delete_test", buffer, 2048);
    std::string created_dir(buffer);

    Context c;
    File f(c, std::string(buffer)), f2(c,std::string(buffer));

    // delete unexisting dir, should fail
   res = f.deletion(&params, &tmp_err);
   assert( res <0);
   assert(tmp_err != NULL);
   assert(tmp_err->getStatus() == StatusCode::FileNotFound);
   DavixError::clearError(&tmp_err);


    // create dir
    res= f.makeCollection(&params, &tmp_err);
    assert(res >=0);
    assert(tmp_err == NULL);


    // test dir
    res= f2.stat(&params, &st, &tmp_err);
    assert(res ==0);
    assert(tmp_err == NULL);
    assert( S_ISDIR(st.st_mode) );

    // delete dir
    res= f.deletion(&params, &tmp_err);
    assert(res >=0);
    assert(tmp_err == NULL);

    return 0;
}

