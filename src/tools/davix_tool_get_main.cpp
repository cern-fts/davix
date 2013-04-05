

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>
#include <errno.h>


// @author : Devresse Adrien
// main file for davix-get operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";



static std::string help_msg(){
    return Tool::get_base_description_options() +
           Tool::get_common_options()+ "\n";
}


static int execute_get(const Tool::OptParams & opts, int out_fd, DavixError** err){
        Context c;
        DavFile f(c, opts.vec_arg[0]);
        return f.getToFd(&opts.params, out_fd, err);
}




int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();
    int out_fd= -1;

    if( (retcode= Tool::parse_davix_get_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){

        if( ( out_fd = Tool::get_output_fstream(opts, scope_get, &tmp_err)) > 0){
            retcode = execute_get(opts, out_fd, &tmp_err);
            close(out_fd);
        }
    }
    Tool::err_display(&tmp_err);
    return retcode;
}







