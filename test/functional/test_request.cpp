#include "test_request.h"

#include <davixcontext.hpp>
#include <neon/neonsessionfactory.hpp>

using namespace Davix;

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }

    DavixError* tmp_err=NULL;
    std::auto_ptr<AbstractSessionFactory> s( new NEONSessionFactory());

    std::auto_ptr<Request> r(s->create_request(argv[1]));
    r->execute_sync(&tmp_err);
    if(tmp_err){
        std::cerr << " error in request : " << tmp_err->getErrMsg() << std::endl;
    }else{
        std::vector<char> v = r->get_result();
        v.push_back('\0');
        std::cout << "content "<< (char*) &(v.at(0)) << std::endl;
    }

    return 0;
}
