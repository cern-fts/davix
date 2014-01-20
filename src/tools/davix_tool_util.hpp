#ifndef DAVIX_TOOL_UTIL_HPP
#define DAVIX_TOOL_UTIL_HPP

#include <cstdio>
#include <cerrno>
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

std::string string_from_mode(mode_t mode);

std::string string_from_ptime(const time_t &t);

// print a string in a minimum of size_string char, fill it with white-space if inferior to this
std::string string_from_size_t(size_t number, size_t size_string);

}
}


#endif // DAVIX_TOOL_UTIL_HPP
