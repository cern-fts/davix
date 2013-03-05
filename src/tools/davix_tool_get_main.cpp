

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>
#include <errno.h>


// @author : Devresse Adrien
// main file for davix get opt


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";



static std::string help_msg(){
    return Tool::get_base_description_options() +
           Tool::get_common_options()+ "\n";
}


static int execute_get(const Tool::OptParams & opts, FILE* fstream, DavixError** err){
        Context c;
        DavPosix pos(&c);
        DAVIX_FD* fd;
        char buffer[READ_BLOCK_SIZE];

        if(  (fd = pos.open(&opts.params, opts.vec_arg[0], O_RDONLY, err))  != NULL){
            ssize_t s_read;
            while( (s_read =  pos.read(fd, buffer, READ_BLOCK_SIZE, err)) > 0){
                if( fwrite(buffer, s_read, 1, fstream) == 0 &&
                        ferror(fstream) != 0){
                    DavixError::setupError(err, scope_get, StatusCode::SystemError, "I/O Error when writing to the destination file ");
                    clearerr(fstream);
                    s_read = -1;
                    break;
                }
            }

            pos.close(fd, NULL);
            fflush(fstream);
            return (s_read==0)?0:-1;
        }
        return -1;
}




int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();
    FILE* fstream= NULL;

    if( (retcode= Tool::parse_davix_get_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){

        if( ( fstream = Tool::configure_fstream(opts, scope_get, &tmp_err)) != NULL){
            retcode = execute_get(opts, fstream, &tmp_err);
            if(fstream != stdout)
                fclose(fstream);
        }
    }
    Tool::err_display(&tmp_err);
    return retcode;
}







