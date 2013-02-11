

#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>


// @author : Devresse Adrien
// main file for davix get opt


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";



static std::string help_msg(){
    return "Usage: %s [OPTIONS ...] <url> \n"
           "Options: \n"
           "\t--capath:         Add an additional certificate authority directory    \n"
           "\t--cred, -E:       Client Certificate in PEM format\n"
           "\t--debug:          Debug mode\n"
           "\t--help, -h:       Display this help message \n"
           "\t--insecure, -k:   Disable SSL credential checks \n"
                       ;
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
    FILE* fstream = stdout;

    if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
        if( (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){
            retcode = execute_get(opts, fstream, &tmp_err);
        }
    }
    Tool::err_display(&tmp_err);
    return retcode;
}







