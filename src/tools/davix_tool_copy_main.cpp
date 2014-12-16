#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>


using namespace Davix;
using namespace std;


const std::string scope_main = "Davix::Tools::davix";


static void performanceCallback(const PerformanceData& perfData, void *udata)
{
    (void) udata;
    std::cout << perfData.totalTransferred()
              << " (" << perfData.avgTransfer() << " bytes/sec)"
              << std::endl;
}


static std::string help_msg(const std::string & cmd_path)(){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_copy_description_options();
    help_msg += Tool::get_common_options();

    return help_msg;
}



int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);

    if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
        Context c;
        if( (retcode = Tool::configureAuth(opts)) == 0){
            configureContext(c, opts);
            DavixCopy copy(c, &opts.params);
            copy.setPerformanceCallback(performanceCallback, NULL);
            copy.copy(opts.vec_arg[0], opts.vec_arg[1],
                      1, &tmp_err);
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}
