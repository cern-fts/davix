#include <davix_internal.hpp>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>

using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_main = "Davix::Tools::davix-mv";



static std::string help_msg(const std::string & cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_base_description_options();
    help_msg += Tool::get_common_options();

    return help_msg;
}



int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);

    TRY_DAVIX{
        if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
            Context c;

            if( (retcode = Tool::configureAuth(opts)) == 0){
                configureContext(c, opts);
                File source(c, opts.vec_arg[0]), dest(c, opts.vec_arg[1]);


                source.move(&opts.params, dest);
                retcode =0;
            }
        }
     }CATCH_DAVIX(&tmp_err){
        retcode = -1;
     }

    Tool::errorPrint(&tmp_err);
    return retcode;
}

