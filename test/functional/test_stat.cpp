#include "test_stat.h"

#include <core.h>
#include <curl/curlsessionfactory.h>
#include <glibmm/init.h>

using namespace Davix;

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }


    try{
        Glib::RefPtr<Core> c= Core::create(new CURLSessionFactory());
        Glib::RefPtr<Stat> stat_ops = c->getStat();
        struct stat st;
        stat_ops->stat(argv[1], &st);

        std::cout << "stat success" << std::endl;
        std::cout << " atime : " << st.st_atime << std::endl;
        std::cout << " mtime : " << st.st_mtime << std::endl;
        std::cout << " ctime : " << st.st_ctime << std::endl;
        std::cout << " mode : " << st.st_mode << std::endl;
        std::cout << " len : " << st.st_size << std::endl;
    }catch(Glib::Error & e){
        std::cout << " error occures : N°" << e.code() << "  " << e.what() << std::endl;
        return -1;
    }catch(std::exception & e){
        std::cout << " error occures :" << e.what() << std::endl;
        return -1;
    }
    return 0;
}

