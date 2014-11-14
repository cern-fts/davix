
#include <cstring>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <davix.hpp>



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
    RequestParams  p;
    dav_ssize_t ret;
    int fd, fd_out, fd_out2;
    Context c;
    const ssize_t size_content = rand()/1000000+2;
    std::cout << " size content" << size_content << std::endl;

    char url[2048];
    generate_random_uri(argv[1], "test_davix_",url, 2048);

    char buffer_mktemp[2048];

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    File f(c, std::string(url));

    fd = open("/dev/urandom", O_RDONLY);
    DAV_ASSERT_TRUE( fd > 0,  "Impossible to open random generator");

    TRY_DAVIX{
        f.put(&p, fd, size_content);
    }CATCH_DAVIX(&tmp_err);

    DAV_ASSERT_TRUE( tmp_err == NULL, tmp_err->getErrMsg());


    strcpy(buffer_mktemp, "dav-fd-test-XXXXXXXXX");
    fd_out = mkstemp(buffer_mktemp);
    DAV_ASSERT_TRUE( fd_out > 0,  "Impossible to output file");
    ret = f.getToFd(&p, fd_out, &tmp_err);
    DAV_ASSERT_TRUE( tmp_err == NULL, tmp_err->getErrMsg());
    DAV_ASSERT_TRUE( ret == size_content, "Invalid size" << ret);

    dav_ssize_t half_size = size_content/2;

    strcpy(buffer_mktemp, "dav-fd-test-partial-XXXXXXXXX");
    fd_out2 = mkstemp(buffer_mktemp);
    DAV_ASSERT_TRUE( fd_out2 > 0,  "Impossible to output file");
    ret = f.getToFd(&p, fd_out2, half_size, &tmp_err);
    DAV_ASSERT_TRUE( tmp_err == NULL, tmp_err->getErrMsg());
    DAV_ASSERT_TRUE( ret == half_size, "Invalid size" << ret);

    lseek(fd_out, 0, 0);
    lseek(fd_out2, 0, 0);

    char buffer1[half_size], buffer2[half_size];

    DAV_ASSERT_TRUE( read(fd_out, buffer1, half_size) == half_size, "Invalid read back from local file 1");
    DAV_ASSERT_TRUE( read(fd_out2, buffer2, half_size) == half_size, "Invalid read back from local file 2");
    DAV_ASSERT_TRUE( memcmp(buffer1, buffer2, half_size) ==0, "Corrupted content");

    close(fd);
    return 0;
}

