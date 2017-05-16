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

#ifndef DAVIX_TOOL_UTIL_HPP
#define DAVIX_TOOL_UTIL_HPP

#include <davix.hpp>
#include <davix_internal.hpp>
#include <tools/davix_tool_params.hpp>


namespace Davix{

namespace Tool{

dav_ssize_t writeToFd(int fd, const void* buffer, dav_size_t s_buff);

dav_ssize_t writeToFd(int fd, const std::string & str);

int configureAuth(OptParams & opts);

bool checkProtocolSanity(OptParams &opts, const std::string &url, DavixError **err);

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

// replace '~' at front of string with env HOME if found
std::string SanitiseTildedPath(const char * path);

bool is_number(const std::string& s);

void TransferMonitor(const Uri & url, Transfer::Type op_type, dav_ssize_t bytes_transfered, dav_size_t total_size, Transfer::Type tool_type);

void printProgressBar(int out_fd, int percent, dav_ssize_t bytes_transfered, dav_size_t total_size, dav_size_t baudrate);

int configureMonitorCB(OptParams & opts, Transfer::Type type);

void tokeniseUrl(std::string url, std::deque<std::string>& dirVec);

// mkdir -p, discard front and back entries if trim is true
int mkdirP(std::string dirPath, bool trim);
int mkdirP(std::deque<std::string>& dirVec, bool trim);

void batchTransferMonitor(std::string dirPath, std::string msg, int entryCount, int totalEntry);

}
}


#endif // DAVIX_TOOL_UTIL_HPP
