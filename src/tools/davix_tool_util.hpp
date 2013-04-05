#ifndef DAVIX_TOOL_UTIL_HPP
#define DAVIX_TOOL_UTIL_HPP

#include <cstdio>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>


namespace Davix{

namespace Tool{


int setup_credential(OptParams & opts, DavixError** err);




void err_display(DavixError ** err);


int DavixToolsAuthCallbackLoginPassword(void* userdata, const SessionInfo & info, std::string & login, std::string & password,
                                        int count, DavixError** err);


// return output file descriptor
int get_output_fstream(const Tool::OptParams & opts, const std::string & scope, DavixError** err);

// return output file descriptor
int get_input_fstream(const Tool::OptParams & opts, const std::string & scope, DavixError** err);

}
}


#endif // DAVIX_TOOL_UTIL_HPP
