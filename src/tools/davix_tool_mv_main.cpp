#include <davix_internal.hpp>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>

using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_main = "Davix::Tools::davix-mv";



static std::string help_msg(){
    return Tool::get_base_description_options() +
            Tool::get_common_options();
}



int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();

    if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
        Context c;

        if( (retcode = Tool::configureAuth(opts)) == 0){
            configureContext(c, opts);
            DavPosix f(&c);
            f.rename(&opts.params, opts.vec_arg[0], opts.vec_arg[1], &tmp_err);
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}

