
#include <davixcontext.hpp>
#include <http_backend.hpp>
#include <posix/davposix.hpp>
#include <fileops/davops.hpp>

#include <string>
#include <sstream>
#include <cmath>
#include <string.h>

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

    RequestParams  p;
    struct stat st;
    int res =-1;
    DavixError* tmp_err=NULL;
    char * base_dir = argv[1];
    char * cert_path = argv[2];
    char buffer[2048];



    if(argc > 2){
        configure_grid_env(cert_path, p);
    }


    srand(time(NULL));
    davix_set_log_level(DAVIX_LOG_ALL);
    generate_random_uri(base_dir, "rmdir_unlink_delete_test", buffer, 2048);
    std::string created_dir(buffer);






    Context c;
    DavPosix pos(&c);
    WebdavQuery query(c);


    std::cout << "verify that " << created_dir << "does not exist" << std::endl;
    res = pos.stat(&p, created_dir, &st, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::FileNotFound);
    DavixError::clearError(&tmp_err);

    std::cout << " verify that unlink() return enoent on not existing dir" << std::endl;
    res = pos.unlink(&p, created_dir, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::FileNotFound);
    DavixError::clearError(&tmp_err);

    std::cout << " verify that rmdir() return enoent on not existing dir" << std::endl;
    res = pos.rmdir(&p, created_dir, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::FileNotFound);
    DavixError::clearError(&tmp_err);

    std::cout << " verify that WebdavQuery::davDelete() return enoent on not existing dir" << std::endl;
    res = query.davDelete(&p, created_dir, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::FileNotFound);
    DavixError::clearError(&tmp_err);

    std::cout << "create  " << created_dir << std::endl;
    res = pos.mkdir(&p, created_dir, 0755, &tmp_err);
    g_assert( res ==0 && tmp_err == NULL);


    std::cout << "verify that   " << created_dir << "exist " << std::endl;
    res = pos.stat(&p, created_dir, &st, &tmp_err);
    g_assert( res == 0);
    g_assert(tmp_err == NULL);
    DavixError::clearError(&tmp_err);

    std::cout << " verify that unlink() does not delete directory " << std::endl;
    res = pos.unlink(&p, created_dir, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::IsADirectory);
    DavixError::clearError(&tmp_err);

    std::cout << " remove dir with rmdir() " << std::endl;
    res = pos.rmdir(&p, created_dir, &tmp_err);
    g_assert( res == 0);
    g_assert(tmp_err == NULL);

    std::cout << "verify that " << created_dir << "does not exist" << std::endl;
    res = pos.stat(&p, created_dir, &st, &tmp_err);
    g_assert( res != 0);
    g_assert(tmp_err && tmp_err->getStatus() == StatusCode::FileNotFound);
    DavixError::clearError(&tmp_err);

    std::cout << "create again " << created_dir << std::endl;
    res = pos.mkdir(&p, created_dir, 0755, &tmp_err);
    g_assert( res ==0 && tmp_err == NULL);


    return 0;
}

