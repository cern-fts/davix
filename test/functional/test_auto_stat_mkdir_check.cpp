#include <davix.hpp>
#include <cstring>
#include <memory>

#include "davix_test_lib.h"

using namespace Davix;


int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage  : " << std::endl;
        std::cout <<"\t" << argv[0] << " [base_dir]" << std::endl;
        std::cout <<"\t" << argv[0] << " [base_dir] [CERTIFICATE_PATH] " << std::endl;
        return 0;
    }

    davix_set_log_level(DAVIX_LOG_ALL);

    DavixError* tmp_err=NULL;
    RequestParams  p;
    std::auto_ptr<Context> c( new Context());
    DavPosix pos(c.get());
    p.setProtocol(RequestProtocol::Webdav);

    if(argc > 2){
        configure_grid_env(argv[2], p);
    }

    char buff[2048];
    generate_random_uri(argv[1], "davix_test_stat", buff, 2048);

    struct stat st;
    // do a first stat, should return enoent
    std::cout << " stat enoent dir " << buff  << std::endl;
    int ret = pos.stat(&p, buff, &st, &tmp_err);
    assert( ret < 0);
    if(tmp_err){
        std::cout << " error " << (int) tmp_err->getStatus() << " msg " << tmp_err->getErrMsg() << std::endl;
    }
    assert(tmp_err && StatusCode::FileNotFound == tmp_err->getStatus());
    DavixError::clearError(&tmp_err);

    std::cout << " create dir " << buff  << std::endl;
    ret = pos.mkdir(&p, buff, 0755, &tmp_err);
    assert(0 == ret);
    assert(NULL == tmp_err);

    std::cout << " stat new dir " << std::endl;
    ret = pos.stat(&p, buff, &st, &tmp_err);
    assert(0 == ret);
    assert(S_ISDIR(st.st_mode));

    std::cout << "stat success" << std::endl;
    std::cout << " atime : " << st.st_atime << std::endl;
    std::cout << " mtime : " << st.st_mtime << std::endl;
    std::cout << " ctime : " << st.st_ctime << std::endl;
    std::cout << " mode : 0" << std::oct << st.st_mode << std::endl;
    std::cout << " len : " << st.st_size << std::endl;

    return 0;
}

