


#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>


// @author : Devresse Adrien
// main file for davix cmd line tool


using namespace Davix;
using namespace std;


static void configure_req(HttpRequest& req, Tool::OptParams & opts){
    req.setParameters(opts.params);
    if(opts.req_type.empty() == false)
        req.setRequestMethod(opts.req_type);

    for(Tool::HeaderVec::iterator it = opts.header_args.begin(); it < opts.header_args.end(); it++){
            req.addHeaderField(it->first, it->second);
    }

}

static std::string help_msg(){
    return "Usage: %s [OPTIONS ...] <url> \n"
           "Options: \n"
           "\t-d, --debug:      Debug mode\n"
           "\t-E, --cred:       Client Certificate in PEM format\n"
           "\t-h, --help:       Display this help message \n"
           "\t-H, --header:     Add a header field to the request (ex: \"Depth: 1\") \n"
           "\t-M, --capath:     Add an additional certificate authority directory    \n"
           "\t-k, --insecure:   Disable SSL credential checks \n"
           "\t-X, --request:    Request operation to use (ex : GET, PUT, PROPFIND, etc..)\n"

                       ;
}

int main(int argc, char** argv){
    int retcode;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();

    if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
        Context c;
        HttpRequest* req;
        if( (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){

            if( (req = c.createRequest(opts.vec_arg[0], &tmp_err)) != NULL){
                configure_req(*req, opts);
                if( (retcode = req->executeRequest(&tmp_err)) ==0){
                    if(req->getAnswerSize() > 0){
                        std::cout << std::string(req->getAnswerContent(),req->getAnswerSize())  << std::endl;
                    }
                }
                delete req;
            }
        }
    }
    if(tmp_err){
        std::cerr << "Error: "<< tmp_err->getErrMsg() << std::endl;
        DavixError::clearError(&tmp_err);
    }
    return retcode;
}







