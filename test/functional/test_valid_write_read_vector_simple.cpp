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
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    srand(time(NULL));

    davix_set_log_level(DAVIX_LOG_ALL);

    DavixError* tmp_err=NULL;
    Davix_fd* fd;
    ssize_t ret=1;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());

    const size_t size_content = rand()/10000000+2000;


    char url[2048];
    generate_random_uri(argv[1], "test_davix_",url, 2048);

    char* buff_input = generate_random_string_content(size_content);


    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    std::cout << " open remote file " << std::endl;

    if( (fd =pos.open(&p, url, O_RDWR | O_CREAT, &tmp_err)) == NULL){
        std::cerr << " open error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    std::cout << " write  content dir " << std::endl;
    // write content
    if( (ret = pos.write(fd, buff_input, size_content, &tmp_err)) < 0){
        std::cerr << " write error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    pos.close(fd, &tmp_err);
    if(tmp_err){
        std::cerr << " error while writing file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }


    if( (fd =pos.open(&p, url, O_RDWR | O_CREAT, &tmp_err)) == NULL){
        std::cerr << " open error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }


    std::cout << " try stupid empty vector read" << std::endl;
    Davix::DavIOVecInput input;
    Davix::DavIOVecOuput output;

    dav_ssize_t res = pos.preadVec(fd, &input, &output, 0, &tmp_err);
    assert(res == 0 && tmp_err == NULL);

    const std::pair<dav_off_t, dav_size_t> range_read = get_random_range_read(size_content);
    char buffer_read[range_read.second+1];
    buffer_read[range_read.second]= '\0';
    input.diov_buffer = buffer_read;
    input.diov_offset = range_read.first;
    input.diov_size = range_read.second;
    std::cout << " try to read one segment of "<< input.diov_size << " from " << input.diov_offset<< " offset of a content of " << size_content << std::endl;
    res = pos.preadVec(fd, &input, &output, 1, &tmp_err);
    assert( res == output.diov_size);
    assert( output.diov_size > 0);
    assert( tmp_err == NULL);
    assert( strncmp(buff_input +input.diov_offset, buffer_read, input.diov_size) ==0);

    if( (ret = pos.close(fd, &tmp_err))){
        std::cerr << " error while closing file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    // delete trash file
    pos.unlink(&p, url, NULL);


    free(buff_input);
    return 0;
}

