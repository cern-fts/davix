/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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


std::string filename_from_uri(const std::string & current_dir, const Uri & uri);

}
}


#endif // DAVIX_TOOL_UTIL_HPP
