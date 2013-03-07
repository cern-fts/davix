#include "test_stat.h"
#include <algorithm>
#include <davix.hpp>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <assert.h>


#include "davix_test_lib.h"

using namespace Davix;


std::pair<dav_off_t, dav_size_t> get_random_range_read(size_t total_size){
    return std::pair<dav_off_t, dav_size_t> ( (rand()%total_size-1), (rand()%total_size)/10+1);
}

int main(int argc, char** argv){
    DavixError* tmp_err=NULL;
    Davix_fd* fd;
    ssize_t ret=1;
    RequestParams  p;
    Context c;
    DavPosix pos(&c);


    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url] " << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH]..." << std::endl;
        return 0;
    }

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }
    srand(time(NULL));

    davix_set_log_level(DAVIX_LOG_ALL);



    size_t vec = 10;
    size_t s_buff = 4096;

    DavIOVecInput in[vec];
    DavIOVecOuput out[vec];
    char buffer[vec][s_buff];
    dav_size_t s[] = { 10000, 5, 20, 12 ,15 ,60, 90, 1, 88, 10};
    dav_off_t off[] = { 10, 20, 2, 10 ,800 ,600, 523, 1, 0, 100};

    for(size_t i = 0; i < vec; ++i){
        in[i].diov_buffer= buffer[i];
        in[i].diov_offset = off[i];
        in[i].diov_size = s[i];
    }

    if( (fd = pos.open(&p, argv[1], O_RDONLY, &tmp_err)) == NULL){
        std::cerr << " open error : err code " << tmp_err->getStatus() << " err msg " << tmp_err->getErrMsg();
        return -1;
    }

    if( ( ret = pos.preadVec(fd, in, out, vec, &tmp_err)) <0){
        std::cerr << " vec read error : err code " << tmp_err->getStatus() << " err msg " << tmp_err->getErrMsg();
        return -1;
    }

    for(size_t i = 0; i < vec; ++i){
        std::cout << "part vector read of size " << out[i].diov_size << " with offset " << in[i].diov_offset << " :" << std::endl;
        std::cout << std::string((const char*) out[i].diov_buffer, out[i].diov_size) << std::endl;
    }


    return 0;
}

