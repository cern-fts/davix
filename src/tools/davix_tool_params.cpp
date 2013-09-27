#include "davix_tool_params.hpp"
#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <logger/davix_logger.h>

namespace Davix{

namespace Tool{


const std::string scope_params = "Davix::Tools::Params";


#define CAPATH_OPT      1000
#define DEBUG_OPT       1001
#define USER_LOGIN      1002
#define USER_PASSWORD   1003
#define DATA_CONTENT    1004
#define S3_SECRET_KEY   1005
#define S3_ACCESS_KEY   1006

// LONG OPTS

#define COMMON_LONG_OPTIONS \
{"verbose", no_argument, 0,  0 }, \
{"debug", no_argument, 0,  DEBUG_OPT }, \
{"version", no_argument, 0, 'V'}, \
{"help", no_argument,0,'?'}

#define SECURITY_LONG_OPTIONS \
{"cert",  required_argument,       0, 'E' }, \
{"capath",  required_argument, 0, CAPATH_OPT }, \
{"userlogin", required_argument, 0, USER_LOGIN}, \
{"userpass", required_argument, 0, USER_PASSWORD}, \
{"s3secretkey", required_argument, 0, S3_SECRET_KEY}, \
{"s3accesskey", required_argument, 0, S3_ACCESS_KEY}, \
{"insecure", no_argument, 0,  'k' }

#define REQUEST_LONG_OPTIONS \
{"header",  required_argument, 0,  'H' }, \
{"request",  required_argument, 0,  'X' }, \
{"data", required_argument, 0, DATA_CONTENT}, \
{"verbose", no_argument, 0,  0 }

OptParams::OptParams() :
    params(),
    vec_arg(),
    verbose(false),
    debug(false),
    req_type(),
    header_args(),
    help_msg(),
    cred_path(),
    output_file_path(),
    input_file_path(),
    userlogpasswd(),
    req_content(),
    aws_auth()
{

}


static void option_abort(char** argv){
    std::cerr << argv[0] <<", Error: Wrong argument format" << std::endl;
    std::cerr << "try 'davix --help' for more informations" <<std::endl;
    exit(-1);
}

static void display_version(){
    std::cout << "Davix Version: " << version() << std::endl;
}


static int set_header_field(const std::string & arg, OptParams & p, DavixError** err){
    dav_size_t pos;
    if( (pos = arg.find(':') ) == std::string::npos){
        DavixError::setupError(err, scope_params, StatusCode::InvalidArgument, " Invalid header field argument");
        return -1;
    }
    p.header_args.push_back(HeaderParam(arg.substr(0,pos), arg.substr(pos+1)));
    return 0;
}



int parse_davix_options_generic(const std::string &opt_filter,
                                const struct option* long_options,
                                int argc, char** argv, OptParams & p, DavixError** err){
    int ret =0;
    int option_index=0;

    while( (ret =  getopt_long(argc, argv, opt_filter.c_str(),
                               long_options, &option_index)) > 0){

        switch(ret){
            case DEBUG_OPT:
                p.debug = true;
                davix_set_log_level(DAVIX_LOG_ALL);
                break;
            case 'E':
                 p.cred_path = std::string(optarg);
                 break;

            case 'k':
                p.params.setSSLCAcheck(false);
                break;
            case 'H':
                if( set_header_field(optarg, p, err) <0)
                    return -1;
                break;
            case CAPATH_OPT:
                p.params.addCertificateAuthorityPath(optarg);
                break;
            case USER_LOGIN:
                p.userlogpasswd.first = optarg;
                break;
            case USER_PASSWORD:
                p.userlogpasswd.second = optarg;
                break;
            case DATA_CONTENT:
                p.req_content = optarg;
                break;
            case S3_ACCESS_KEY:
                p.aws_auth.second = optarg;
                break;
            case S3_SECRET_KEY:
                p.aws_auth.first = optarg;
                break;
            case 'o':
                p.output_file_path = optarg;
                break;
            case 'V':
                display_version();
                return 1;

            case 'X':
                p.req_type = std::string(optarg, 0, 255);
            break;
            case '?':
            printf(p.help_msg.c_str(), argv[0]);
                return -1;
          break;
            default:
                option_abort(argv);
        }
    }


   ret =-1;
   for(int i = optind; i < argc; ++i){
            p.vec_arg.push_back(argv[i]);
            ret =0;
    }

    if(ret != 0){
        option_abort(argv);
    }

    return ret;
}



int parse_davix_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "H:E:X:o:kV";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        REQUEST_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    return parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err);
}


int parse_davix_ls_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "E:vkV";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) <0
            || p.vec_arg.size() != 1){
        option_abort(argv);
        return -1;
    }
    return 0;
}


int parse_davix_get_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "E:o:vkV";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) < 0
            || p.vec_arg.size() != 1){
        option_abort(argv);
        return -1;
    }
    return 0;
}

int parse_davix_put_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "E:o:vkV";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };



    if( parse_davix_options_generic(arg_tool_main,
                                    long_options,
                                    argc,
                                    argv,
                                    p, err) < 0
        || p.vec_arg.size() != 2){

        option_abort(argv);
        return -1;
    }
    p.input_file_path = p.vec_arg[0];
    return 0;
}


const std::string  & get_common_options(){
    static const std::string s(
            "  Common Options:\n"
            "\t--debug:                  Debug mode\n"
            "\t--help, -h:               Display this help message\n"
            "\t--output, -o file:        Redirect output to file\n"
            "\t--verbose:                Verbose mode\n"
            "\t--version, -V:            Display version\n"
            "  Security Options:\n"
            "\t--capath CA_path:         Add an additional certificate authority directory\n"
            "\t--cred, -E cred_path:     Client Certificate in PEM format\n"
            "\t--insecure, -k:           Disable SSL credential checks\n"
            "\t--userlogin:              User login for login/password authentication\n"
            "\t--userpass:               User password for login/password authentication\n"
            "\t--s3secretkey:            S3 authentication: secret key\n"
            "\t--s3accesskey:            S3 authentication: access key\n"
            );
    return s;
}

const std::string  & get_base_description_options(){
    static const std::string s("Usage: %s [OPTIONS ...] <url> \n"
            );
    return s;
}

const std::string  & get_put_description_options(){
    static const std::string s("Usage: %s [OPTIONS ...] <local_file> <remote_file_url> \n"
            );
    return s;
}



}

}
