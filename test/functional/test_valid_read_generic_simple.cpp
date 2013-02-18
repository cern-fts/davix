#include "test_stat.h"

#include <davix.hpp>
#include <cstring>
#include <memory>


#include "davix_test_lib.h"

using namespace Davix;


int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);

    DavixError* tmp_err=NULL;
    Davix_fd* fd;
    ssize_t ret=1;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());
    size_t char_counter=0, chunk_counter=0;

    char buff[2049]={0};

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    if( (fd =pos.open(&p, argv[1], O_RDONLY, &tmp_err)) == NULL){
        std::cerr << " error while opening file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

   // std::cout << "file content " << std::endl;
    while( fd && (ret = pos.read(fd, buff, 2048, &tmp_err)) > 0){
        buff[ret] = '\0';
        char_counter += strlen(buff);
        chunk_counter += ret;
        if(char_counter != chunk_counter){
            std::cerr << " char size and read size are differents.. " << std::endl;
            return -1;
        }
        std::cout << buff;
    }
    std::cout << std::endl;

    if(tmp_err){
        std::cerr << " error while readding file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    if( (ret = pos.close(fd, &tmp_err))){
        std::cerr << " error while closing file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    return 0;
}

