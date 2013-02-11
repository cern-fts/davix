

#include <cstdio>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>


// @author : Devresse Adrien
// main file for davix ls tool


using namespace Davix;
using namespace std;


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


static void display_dirent_entry(struct dirent* d, const Tool::OptParams & opts, FILE* filestream){
    fputs(d->d_name, filestream);
    fputs("\n",filestream);
}

/*
static void display_long_dirent_entry(struct dirent* d, struct stat* st, const Tool::OptParams & opts, FILE* filestream){
    fprintf(filestream, "%s %d\t %d\t %s",d->d_name, st->st_nlink, st->st_size, )
}*/

static void display_path(const std::string & str, FILE* filestream){
    fputs(str.c_str(), filestream);
    fputs("\n",filestream);
}

static int listing(const Tool::OptParams & opts, FILE* filestream, DavixError** err ){
    DAVIX_DIR* fd = NULL;
    Context c;
    DavPosix pos(&c);
    struct stat st;
    struct dirent* d;
    if( (fd = pos.opendirpp(&opts.params, opts.vec_arg[0], err)) == NULL)
        return -1;
    while( (d = pos.readdirpp(fd, &st, err)) != NULL ){
        display_dirent_entry(d, opts, filestream);
    }
    pos.closedirpp(fd, NULL);
    return (err && *err)?(-1):0;
}

static int get_info(const Tool::OptParams & opts, FILE* filestream, DavixError** err ){
    Context c;
    DavPosix pos(&c);
    struct stat st;
    if( pos.stat(&opts.params, opts.vec_arg[0], &st, err) == 0){
        display_path(opts.vec_arg[0], filestream);
        return 0;
    }
    return -1;
}




int main(int argc, char** argv){
    int retcode;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();
    FILE* fstream = stdout;

    if( (retcode= Tool::parse_davix_ls_options(argc, argv, opts, &tmp_err)) ==0){
        if( (retcode = Tool::setup_credential(opts, &tmp_err)) == 0){
            retcode = listing(opts, fstream, &tmp_err);
            if(retcode < 0 && tmp_err->getStatus() == StatusCode::IsNotADirectory){
                DavixError::clearError(&tmp_err);
                retcode = get_info(opts, fstream, &tmp_err);
            }
        }
    }
    Tool::err_display(&tmp_err);
    return retcode;
}







