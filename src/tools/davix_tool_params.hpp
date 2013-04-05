#ifndef DAVIX_TOOL_PARAMS_HPP
#define DAVIX_TOOL_PARAMS_HPP


#include <vector>
#include <string>
#include <davix.hpp>

namespace Davix{

namespace Tool{

typedef std::pair<std::string, std::string> HeaderParam;
typedef std::pair<std::string, std::string> LoginPasswd;
typedef std::vector<HeaderParam> HeaderVec;

struct OptParams{
    OptParams();
    RequestParams params;
    // vector of non-option arguments in order
    std::vector<std::string> vec_arg;
    int verbose;
    int debug;
    // request command
    std::string req_type;
    // request header
    HeaderVec header_args;
    // help msg
    std::string help_msg;
    // credential path
    std::string cred_path;
    // output file -o
    std::string output_file_path;
    // input file path
    std::string input_file_path;
    // user  login/passwd
    LoginPasswd userlogpasswd;
    // request content
    std::string req_content;
    // s3 auth
    std::pair<std::string, std::string> aws_auth;
};

int parse_davix_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_ls_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_get_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_put_options(int argc, char** argv, OptParams & p, DavixError** err);



const std::string & get_common_options();

const std::string & get_base_description_options();

const std::string  & get_put_description_options();

}

}

#endif // DAVIX_TOOL_PARAMS_HPP
