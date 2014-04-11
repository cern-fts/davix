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

#include <davix.hpp>
#include <davix_internal.hpp>
#include <tools/davix_tool_params.hpp>


namespace Davix{

namespace Tool{

dav_ssize_t writeToFd(int fd, const void* buffer, dav_size_t s_buff);

dav_ssize_t writeToFd(int fd, const std::string & str);

int configureAuth(OptParams & opts, DavixError** err);

void configureContext(Context &context, const OptParams & opts);

void errorPrint(DavixError ** err);

void writeConsoleLine(int fd, char symbol, const std::string & msg);

int authCallbackLoginPassword(void* userdata, const SessionInfo & info, std::string & login, std::string & password,
                                        int count, DavixError** err);

int authCallbackCert(void* userdata, const SessionInfo & info, X509Credential* cert, DavixError** err);

// return output file descriptor
int getOutFd(const Tool::OptParams & opts, const std::string & scope, DavixError** err);

// return output file descriptor
int getInFd(const Tool::OptParams & opts, const std::string & scope, DavixError** err);

// string utils
std::string string_from_mode(mode_t mode);
std::string string_from_ptime(const time_t &t);

// print a string in a minimum of size_string char, fill it with white-space if inferior to this
std::string string_from_size_t(size_t number, size_t size_string);

std::string filename_from_uri(const std::string & current_dir, const Uri & uri);

// shell related utils
bool isShell(int fd);
void flushFinalLineShell(int fd);

}
}


#endif // DAVIX_TOOL_UTIL_HPP
