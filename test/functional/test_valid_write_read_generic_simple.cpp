#include "test_stat.h"

#include <davix.hpp>
#include <cstring>
#include <memory>
#include <algorithm>
#include <cstdlib>


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

    DavixError* tmp_err=NULL;
    Davix_fd* fd;
    ssize_t ret=1;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
  //  c->setSessionCaching(false);
    DavPosix pos(c.get());

    const size_t size_content = rand()/1000000+2;


    char url[2048];
    generate_random_uri(argv[1], "test_davix_",url, 2048);

    char buff_output[size_content+1];
    char* buff_input = generate_random_string_content(size_content);


    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    if( (fd =pos.open(&p, url, O_RDONLY, &tmp_err)) != NULL){
        std::cerr << "Error ! open should fail, not an existing file and read only mode " << std::endl;
        return -1;
    }
    DavixError::clearError(&tmp_err);

    if( (fd =pos.open(&p, url, O_RDWR, &tmp_err)) != NULL){
        std::cerr << "Error ! open should fail, not an existing file and  not O_CREAT " << std::endl;
        return -1;
    }
    DavixError::clearError(&tmp_err);

    if( (fd =pos.open(&p, url, O_RDWR | O_CREAT, &tmp_err)) == NULL){
        std::cerr << " open error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    // write content
    if( (ret = pos.write(fd, buff_input, size_content, &tmp_err)) < 0){
        std::cerr << " write error "<< tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    if( pos.lseek(fd, 0,0, &tmp_err) != 0){
        std::cerr << " error while lseek file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

   // read content back !
    off_t offset_buffer = 0;
    while(  (ret = pos.read(fd, buff_output+ offset_buffer, 50, &tmp_err) ) > 0
            && (size_t) offset_buffer  < size_content){
        offset_buffer += ret;
    }

    if(tmp_err){
        std::cerr << " error while readding file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    buff_output[size_content] ='\0';
    if(strncmp(buff_input, buff_output, size_content) !=0){
        std::cerr << "content are different : FATAL ! " << std::endl;
        return -1;
    }




    // try partial read
    memset(buff_output, 0, sizeof(size_content));
    off_t offset_read = size_content/2;
    size_t size_read = std::min<size_t>(size_content - offset_read, 600);
    if( ( ret = pos.pread(fd, buff_output, size_read, offset_read, &tmp_err)) <0){
        std::cerr << " error while pread " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }
    buff_output[size_read] ='\0';
    if(strncmp(buff_input+offset_read, buff_output, size_read) !=0){
        std::cerr << "content are different : FATAL ! " << std::endl;
        return -1;
    }


    if( (ret = pos.close(fd, &tmp_err))){
        std::cerr << " error while closing file " << tmp_err->getErrMsg() << " code :" << (int) tmp_err->getStatus() << std::endl;
        return -1;
    }

    if( (fd =pos.open(&p, url, O_RDWR | O_CREAT | O_EXCL, &tmp_err)) != NULL){
        std::cerr << " should fail , O_EXCL flag and file exist" << std::endl;
        return -1;
    }

    free(buff_input);
    return 0;
}

