#include "test_request.h"

#include <davix.hpp>

using namespace Davix;

int main(int argc, char** argv){
    if( argc < 2){
        std::cout << "Usage " << argv[0] << " [url]" << std::endl;
        return 0;
    }

    DavixError* tmp_err=NULL;
    Context c;
    HttpRequest r(c, argv[1], &tmp_err);

    if(!tmp_err)
        r.executeRequest(&tmp_err);
    if(tmp_err){
        std::cerr << " error in request : " << tmp_err->getErrMsg() << std::endl;
    }else{
        std::vector<char> body = r.getAnswerContentVec();
        std::string v(body.begin(), body.end());
        std::cout << "content "<< v << std::endl;
    }

    return 0;
}
