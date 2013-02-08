#ifndef DAVIX_TOOL_PARAMS_HPP
#define DAVIX_TOOL_PARAMS_HPP


#include <vector>
#include <string>
#include <davix.hpp>

namespace Davix{

namespace Tool{

typedef std::pair<std::string, std::string> HeaderParam;
typedef std::vector<HeaderParam> HeaderVec;

struct OptParams{
    OptParams();
    RequestParams params;
    std::vector<std::string> vec_arg;
    int verbose;
    int debug;
    std::string req_type;
    HeaderVec header_args;
    std::string help_msg;
    std::string cred_path;
};

int parse_davix_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_ls_options(int argc, char** argv, OptParams & p, DavixError** err);


}

}

#endif // DAVIX_TOOL_PARAMS_HPP
