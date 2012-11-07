#include "test_stat.h"

#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davposix.hpp>
#include <string.h>
#include <cstdlib>


#include "davix_test_lib.h"

using namespace Davix;



static void configure_grid_env(char * cert_path, RequestParams&  p){
    p.setSSLCAcheck(false);
    p.setAuthentificationCallback(cert_path, &mycred_auth_callback);
}


int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [url]" << std::endl;
        std::cout <<"\t" << argv[0] << " [url] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    g_logger_set_globalfilter(G_LOG_LEVEL_DEBUG);

    DavixError* tmp_err=NULL;
    Davix_fd* fd;
    ssize_t ret=1;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());

    const size_t size_content = rand()/1000000+2;

    char buff_output[size_content+1];
    char* buff_input = generate_random_string_content(size_content);


    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    if( (fd =pos.open(&p, argv[1], O_RDONLY, &tmp_err)) == NULL){
        std::cerr << " error while opening file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    // write content
    if( (ret = pos.write(fd, buff_input, size_content, &tmp_err)) < 0){
        std::cerr << " write error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }


   // read content back !
    off_t offset_buffer = 0;
    while(  (ret = pos.read(fd, buff_output, 2048, &tmp_err) ) > 0
            && (size_t) offset_buffer  < size_content){
        offset_buffer += ret;
    }
    buff_output[size_content] ='\0';
    if(strncmp(buff_input, buff_output, size_content) !=0){
        std::cerr << "content are different : FATAL ! " << std::endl;
        return -1;
    }


    if(tmp_err){
        std::cerr << " error while readding file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    if( (ret = pos.close(fd, &tmp_err))){
        std::cerr << " error while closing file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    free(buff_input);
    return 0;
}

