#include <davix_internal.hpp>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>


// @author : Devresse Adrien
// main file for davix low level cmd line tool


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_main = "Davix::Tools::davix";



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
        if( (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){
            DavFile f(c,opts.vec_arg[0]);
            f.deletion(&opts.params, &tmp_err);
        }
    }
    Tool::err_display(&tmp_err);
    return retcode;
}







