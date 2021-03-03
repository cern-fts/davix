/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
#include "davix_tool_params.hpp"
#include "davix_tool_util.hpp"
#include "davix_config_parser.hpp"
#include <getopt.h>
#include <utils/stringutils.hpp>
#include <utils/davix_logger.hpp>

namespace Davix{

namespace Tool{


const std::string scope_params = "Davix::Tools::Params";


#define CAPATH_OPT             1000
#define DEBUG_OPT              1001
#define USER_LOGIN             1002
#define USER_PASSWORD          1003
#define DATA_CONTENT           1004
#define S3_SECRET_KEY          1005
#define S3_ACCESS_KEY          1006
#define X509_PRIVATE_KEY       1007
#define TRACE_OPTIONS          1008
#define REDIRECTION_OPT        1009
#define METALINK_OPT           1010
#define CONN_TIMEOUT           1011
#define TIMEOUT_OPS            1012
#define RETRY_OPT              1013
#define S3_LISTING_MODE        1014
#define S3_MAX_KEYS            1015
#define RETRY_DELAY_OPT        1016
#define HAS_INPUT_FILE         1017
#define THIRD_PT_COPY_MODE     1018
#define DISABLE_LISTING_CAP    1019
#define S3_REGION              1020
#define S3_ALTERNATE           1021
#define AZURE_KEY              1022
#define S3_TOKEN               1023
#define NO_100_CONTINUE        1024
#define ACCEPTED_RETRY         1025
#define ACCEPTED_RETRY_DELAY   1026
#define GCLOUD_CRED_PATH       1027
#define OS_TOKEN               1028
#define OS_PROJECT_ID          1029
#define SWIFT_LISTING_MODE     1030
#define SWIFT_ACCOUNT          1031

// LONG OPTS

#define COMMON_LONG_OPTIONS \
{"debug", no_argument, 0,  DEBUG_OPT }, \
{"header",  required_argument, 0,  'H' }, \
{"help", no_argument, 0,'?'}, \
{"metalink", required_argument, 0, METALINK_OPT }, \
{"module", required_argument, 0, 'P'}, \
{"proxy", required_argument, 0, 'x'}, \
{"redirection", required_argument, 0, REDIRECTION_OPT }, \
{"conn-timeout", required_argument, 0, CONN_TIMEOUT }, \
{"retry", required_argument, 0, RETRY_OPT }, \
{"retry-delay", required_argument, 0, RETRY_DELAY_OPT }, \
{"timeout", required_argument, 0, TIMEOUT_OPS }, \
{"trace", required_argument, 0, TRACE_OPTIONS }, \
{"infile", required_argument, 0, HAS_INPUT_FILE }, \
{"version", no_argument, 0, 'V'}

#define SECURITY_LONG_OPTIONS \
{"cert",  required_argument,       0, 'E' }, \
{"capath",  required_argument, 0, CAPATH_OPT }, \
{"key", required_argument, 0, X509_PRIVATE_KEY}, \
{"userlogin", required_argument, 0, USER_LOGIN}, \
{"userpass", required_argument, 0, USER_PASSWORD}, \
{"s3secretkey", required_argument, 0, S3_SECRET_KEY}, \
{"s3accesskey", required_argument, 0, S3_ACCESS_KEY}, \
{"s3region", required_argument, 0, S3_REGION}, \
{"s3token", required_argument, 0, S3_TOKEN}, \
{"azurekey", required_argument, 0, AZURE_KEY}, \
{"s3alternate", no_argument, 0, S3_ALTERNATE}, \
{"gcloud-creds", required_argument, 0, GCLOUD_CRED_PATH}, \
{"insecure", no_argument, 0,  'k' }, \
{"ostoken", required_argument, 0, OS_TOKEN}, \
{"osprojectid", required_argument, 0, OS_PROJECT_ID}, \
{"swiftaccount", required_argument, 0, SWIFT_ACCOUNT}

#define REQUEST_LONG_OPTIONS \
{"request",  required_argument, 0,  'X' }, \
{"data", required_argument, 0, DATA_CONTENT}, \
{"verbose", no_argument, 0,  0 }

#define LISTING_LONG_OPTIONS \
{"s3-listing", required_argument, 0,  S3_LISTING_MODE }, \
{"s3-maxkeys", required_argument, 0,  S3_MAX_KEYS }, \
{"no-cap", required_argument, 0, DISABLE_LISTING_CAP}, \
{"long-list", no_argument, 0,  'l' }, \
{"swift-listing", required_argument, 0, SWIFT_LISTING_MODE}

#define GET_LONG_OPTIONS \
{"accepted-retry", required_argument, 0, ACCEPTED_RETRY}, \
{"accepted-retry-delay", required_argument, 0, ACCEPTED_RETRY_DELAY}

#define PUT_LONG_OPTIONS \
{"no-100-continue", no_argument, 0,  NO_100_CONTINUE }

#define COPY_LONG_OPTIONS \
{"copy-mode", required_argument, 0,  THIRD_PT_COPY_MODE }

OptParams::OptParams() :
    params(),
    vec_arg(),
    debug(false),
    s3_delete_per_request(20),
    threadpool_size(10),
    req_type(),
    help_msg(),
    cred_path(),
    priv_key(),
    cred_pass(),
    output_file_path(),
    input_file_path(),
    userlogpasswd(),
    userlogpasswd_through_cmdline(false),
    req_content(),
    aws_auth(),
    aws_region(),
    aws_token(),
    aws_alternate(false),
    os_token(),
    os_project_id(),
    swift_account(),
    pres_flag(0),
    shell_flag(0),
    has_input_file(false),
    no_cap(false)
{

}


static void option_abort(char** argv){
    std::cerr << argv[0] <<", Error: Wrong argument format\n";
    std::cerr << "Try '" << argv[0] <<" --help' for more information" << std::endl;
    exit(-1);
}

static void display_version(){
    std::cout << "Version: " << version() << std::endl;
    std::cout << "Runtime curl version: " << backendRuntimeVersion() << std::endl;
    std::cout << "Compiled against curl " << backendHeadersVersion() << std::endl;

    exit(0);
}

template <typename T, typename Y, typename Z>
Y match_option(T begin, T end,
               Y begin_res, Y end_res,
               Z val, char** argv){
    Y res = match_array(begin, end, begin_res, end_res, val);
    if(res == end_res){
        option_abort(argv);
    }
    return res;
}

static int set_header_field(const std::string & arg, OptParams & p, DavixError** err){
    dav_size_t pos;
    if( (pos = arg.find(':') ) == std::string::npos){
        DavixError::setupError(err, scope_params, StatusCode::InvalidArgument, " Invalid header field argument");
        return -1;
    }

    dav_size_t value_pos = pos+1;
    if(arg.size() > pos+1 && arg[pos+1] == ' ') {
        value_pos = pos+2;
    }
    p.params.addHeader(arg.substr(0,pos), arg.substr(value_pos));
    return 0;
}

static void set_metalink_opt(RequestParams & params, const std::string & metalink_opt, char** argv){
    const std::string str_opt[] = { "no" , "disable", "auto", "failover" };
    const Davix::MetalinkMode::MetalinkMode mode_opt[] = { MetalinkMode::Disable, MetalinkMode::Disable, MetalinkMode::Auto, MetalinkMode::FailOver };
    params.setMetalinkMode(*match_option(str_opt, str_opt+sizeof(str_opt)/sizeof(str_opt[0]),
                                               mode_opt, mode_opt + sizeof(mode_opt)/sizeof(mode_opt[0]),
                                               metalink_opt, argv));
}


static void set_redirection_opt(RequestParams & params, const std::string & redir_opt, char** argv){
    const std::string str_opt[] = { "no" , "yes", "auto"};
    const bool mode_opt[] = { false, true, true };
    params.setTransparentRedirectionSupport(*match_option(str_opt, str_opt+sizeof(str_opt)/sizeof(str_opt[0]),
                                               mode_opt, mode_opt + sizeof(mode_opt)/sizeof(mode_opt[0]),
                                               redir_opt, argv));
}


static int parse_int(const std::string & opt, char** argv){
    try{
        return toType<int, std::string>()(opt);
    }catch(...){
        std::cerr << "Invalid option value " << opt << std::endl;
        option_abort(argv);
    }
    return 0;
}

static struct timespec parse_timeout(const std::string & opt, char** argv){
    int t = parse_int(opt, argv);

    struct timespec timelapse;
    timelapse.tv_sec =t;
    timelapse.tv_nsec =0;
    return timelapse;
}

void parse_davix_config(OptParams &p, std::string url) {
    if(davix_config_apply("~/.davixrc", p, url)) return;
    if(davix_config_apply("~/.netrc", p, url)) return;
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
                // max log level
                setLogLevel(DAVIX_LOG_TRACE);
                setLogScope(DAVIX_LOG_SCOPE_ALL ^ DAVIX_LOG_XML);
                break;
            case 'E':
                 p.cred_path = SanitiseTildedPath(optarg);
                 break;
            case 'k':
                p.params.setSSLCAcheck(false);
                break;
            case 'H':
                if( set_header_field(optarg, p, err) <0)
                    return -1;
                break;
            case S3_LISTING_MODE:
                {
                    if(std::string(optarg).compare("flat")==0)
                        p.params.setS3ListingMode(S3ListingMode::Flat);
                    else if(std::string(optarg).compare("hierarchical")==0)
                        p.params.setS3ListingMode(S3ListingMode::Hierarchical);
                    else if(std::string(optarg).compare("semi")==0)
                        p.params.setS3ListingMode(S3ListingMode::SemiHierarchical);
                    break;
                }
            case S3_MAX_KEYS:
                p.params.setS3MaxKey(atoi(optarg));
                break;
            case SWIFT_LISTING_MODE:
                {
                    if(std::string(optarg).compare("hierarchical")==0)
                        p.params.setSwiftListingMode(SwiftListingMode::Hierarchical);
                    else if(std::string(optarg).compare("semi")==0)
                        p.params.setSwiftListingMode(SwiftListingMode::SemiHierarchical);
                    break;
                }
            case CAPATH_OPT:
                p.params.addCertificateAuthorityPath(optarg);
                break;
            case USER_LOGIN:
                p.userlogpasswd.first = optarg;
                p.userlogpasswd_through_cmdline = true;
                strncpy(optarg, "", strlen(optarg));
                break;
            case X509_PRIVATE_KEY:
                p.priv_key = SanitiseTildedPath(optarg).c_str();
                break;
            case USER_PASSWORD:
                p.userlogpasswd.second = optarg;
                p.userlogpasswd_through_cmdline = true;
                strncpy(optarg, "", strlen(optarg));
                break;
            case DATA_CONTENT:
                p.req_content = optarg;
                break;
            case RETRY_OPT:
                p.params.setOperationRetry( parse_int(optarg, argv));
                break;
            case RETRY_DELAY_OPT:
                p.params.setOperationRetryDelay( parse_int(optarg, argv));
                break;
            case S3_ACCESS_KEY:
                p.aws_auth.second = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case S3_SECRET_KEY:
                p.aws_auth.first = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case S3_REGION:
                p.aws_region = optarg;
                break;
            case S3_TOKEN:
                p.aws_token = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case S3_ALTERNATE:
                p.aws_alternate = true;
                break;
            case AZURE_KEY:
                p.azure_key = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case GCLOUD_CRED_PATH:
                p.gcloud_creds_path = SanitiseTildedPath(optarg).c_str();
                break;
            case OS_TOKEN:
                p.os_token = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case OS_PROJECT_ID:
                p.os_project_id = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case SWIFT_ACCOUNT:
                p.swift_account = optarg;
                strncpy(optarg, "", strlen(optarg));
                break;
            case 'l':
                p.pres_flag |= LONG_LISTING_FLAG;
                break;
            case 'o':
                p.output_file_path = optarg;
                break;
            case 'P':
                p.modules_list = StrUtil::tokenSplit(std::string(optarg), ",");
                break;
            case 'V':
                display_version();
                return 1;
            case TRACE_OPTIONS:
                {
                    p.debug = true;
                    setLogScope(0);
                    setLogScope(std::string(optarg));
                    setLogLevel(DAVIX_LOG_TRACE);
                }
                break;
            case 'x':
                p.params.setProxyServer(std::string(optarg, 0, 2048));
                break;
            case 'X':
                p.req_type = std::string(optarg, 0, 255);
                break;
            case METALINK_OPT:
                 set_metalink_opt(p.params, std::string(optarg), argv);
                 break;
            case REDIRECTION_OPT:
                 set_redirection_opt(p.params, std::string(optarg), argv);
                 break;
            case CONN_TIMEOUT:{
                struct timespec s =parse_timeout(std::string(optarg), argv);
                p.params.setConnectionTimeout(&s);
                }
                break;
            case TIMEOUT_OPS:{
                struct timespec s =parse_timeout(std::string(optarg), argv);
                p.params.setOperationTimeout(&s);
                break;
             }
            case HAS_INPUT_FILE:{
                p.has_input_file = true;
                p.input_file_path = optarg;
                }
                break;
            case THIRD_PT_COPY_MODE:{
                if(std::string(optarg).compare("pull")==0)
                    p.params.setCopyMode(CopyMode::Pull);
                else if(std::string(optarg).compare("push")==0)
                    p.params.setCopyMode(CopyMode::Push);
                break;
                }
            case DISABLE_LISTING_CAP:{
                p.no_cap = true;
                break;
                }
            case 'r':{
                p.params.setRecursiveMode(true);
                p.threadpool_size = atoi(optarg);
                break;
                }
            case 'n':{
                p.s3_delete_per_request = (atoi(optarg));
                break;
                }
            case NO_100_CONTINUE:{
                p.params.set100ContinueSupport(false);
                break;
            }
            case ACCEPTED_RETRY:
                std::cout << "in accepted retry" << std::endl;
                p.params.setAcceptedRetry(atoi(optarg));
                break;
            case ACCEPTED_RETRY_DELAY:
                p.params.setAcceptedRetryDelay(atoi(optarg));
                break;
            case '?':
                std::cout <<  p.help_msg;
                exit(1);
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
    const std::string arg_tool_main= "P:x:H:E:X:o:kVr:";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        REQUEST_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) < 0) {
        return -1;
    }

    parse_davix_config(p, p.vec_arg[0]);
    return 0;
}


int parse_davix_ls_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "P:x:H:E:vkVlfr:";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        LISTING_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) <0
            || p.vec_arg.size() != 1){
        option_abort(argv);
        return -1;
    }
    parse_davix_config(p, p.vec_arg[0]);
    return 0;
}


int parse_davix_get_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "P:x:H:E:o:OvkVr:";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        GET_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) < 0
            || p.vec_arg.size() > 2){
        option_abort(argv);
        return -1;
    }

    if(p.vec_arg.size() == 2){
        p.output_file_path = p.vec_arg[1];
    }
    parse_davix_config(p, p.vec_arg[0]);
    return 0;
}

int parse_davix_put_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "P:x:H:E:o:vkVr:";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        PUT_LONG_OPTIONS,
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
    parse_davix_config(p, p.vec_arg[1]);
    return 0;
}

int parse_davix_copy_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "P:x:H:E:X:o:kV";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        REQUEST_LONG_OPTIONS,
        COPY_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if(parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) < 0) {
        return -1;
    }

    parse_davix_config(p, p.vec_arg[0]);
    return 0;
}

int parse_davix_rm_options(int argc, char** argv, OptParams & p, DavixError** err){
    const std::string arg_tool_main= "P:x:H:E:X:o:kVr:n:";
    const struct option long_options[] = {
        COMMON_LONG_OPTIONS,
        SECURITY_LONG_OPTIONS,
        REQUEST_LONG_OPTIONS,
        {0,         0,                 0,  0 }
     };

    if( parse_davix_options_generic(arg_tool_main, long_options,
                                       argc, argv,
                                       p, err) < 0) {
        return -1;
    }

    parse_davix_config(p, p.vec_arg[0]);
    return 0;
}

std::string get_common_options(){
           return  "  Common Options:\n"
            "\t--conn-timeout TIME:      Connection timeout in seconds. default: 30\n"
            "\t--retry NUMBER:           Number of retry attempts in case of an operation failure. default: 3\n"
            "\t--retry-delay TIME:       Number of seconds to wait between retry attempts. default: 0\n"
            "\t--debug:                  Debug mode\n"
            "\t--header, -H:             Add a header field to the request\n"
            "\t--help, -h:               Display this help message\n"
            "\t--metalink OPT:           Metalink support. value=failover|no. default=failover) \n"
            "\t--module, -P NAME:        Load a plugin or profile by name\n"
            "\t--proxy, -x URL:          SOCKS5 proxy server URL. (Ex: socks5://login:pass@socks.example.org)\n"
            "\t--redirection OPT:        Transparent redirection support. value=yes|no. default=yes)\n"
            "\t--timeout TIME:           Global timeout for the operation in seconds. default: infinite\n"
            "\t--trace:                  Specify one or more scopes to trace. (Ex: --trace log level(optional),header,file)\n"
            "\t--version, -V:            Display version\n"
            "  Security Options:\n"
            "\t--capath CA_PATH:         Add an additional certificate authority directory\n"
            "\t--cert, -E CRED_PATH:     Client Certificate in PEM format\n"
            "\t--key KEY_PATH:           Private key in PEM format\n"
            "\t--insecure, -k:           Disable SSL credential checks\n"
            "\t--userlogin:              User login for login/password authentication\n"
            "\t--userpass:               User password for login/password authentication\n"
            "\t--s3secretkey SEC_KEY:    S3 authentication: secret key\n"
            "\t--s3accesskey ACC_KEY:    S3 authentication: access key\n"
            "\t--s3region REGION:        S3 region (optional - if passed, will authenticate using a v4 signature instead of v2)\n"
            "\t--s3token TOKEN:          S3 security token - used along with STS temporary credentials\n"
            "\t--s3alternate:            Pass this flag if you're using a path-based S3 URL\n"
            "\t                          A path-based URL contains the bucket name in the path, ie https://s3-someregion.amazonaws.com/mybucket/file\n"
            "\t--azurekey AZURE_KEY:     Azure authentication secret key\n"
            "\t--gcloud-creds PATH:      Path to gcloud json credentials\n"
            "\t--ostoken TOKEN:          Swift authentication: Openstack token\n"
            "\t--osprojectid:            Swift authentication: Openstack project ID\n"
            "\t--swiftaccount:           Alternative Swift authentication: Swift account\n"
            ;
}



std::string  get_base_description_options(){
    return "[OPTIONS ...] <url>\n"
            ;
}

std::string  get_get_description_options(){
    return "[OPTIONS ...] <url> [local_file]\n"
            ;
}



std::string get_put_description_options(){
    return "[OPTIONS ...] <local_file> <remote_file_url> \n"
            ;
}


std::string get_copy_description_options(){
    return "[OPTIONS ...] <src_url> <dst_url>\n"
            ;
}

}

}
