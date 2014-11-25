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


#ifndef DAVIX_LOGGER_HPP
#define DAVIX_LOGGER_HPP

#include <cstdarg>
#include <utils/davix_types.hpp>

DAVIX_C_DECL_BEGIN

#define DAVIX_LOG_CRITICAL  1
#define DAVIX_LOG_WARNING   2
#define DAVIX_LOG_VERBOSE   3
#define DAVIX_LOG_DEBUG     4
#define DAVIX_LOG_TRACE     5
#define DAVIX_LOG_ALL       6

#define LOG_FILE       (1<<0)
#define LOG_POSIX      (1<<1)
#define LOG_XML        (1<<2)
#define LOG_SSL        (1<<3)
#define LOG_HEADER     (1<<4)
#define LOG_BODY       (1<<5)
#define LOG_CHAIN      (1<<6)
#define LOG_CORE       (1<<7)
#define LOG_GRID       (1<<8)
#define LOG_SOCKET     (1<<9)
#define LOG_LOCKS      (1<<10)
#define LOG_SCOPE_NEON (1<<29)
#define LOG_ALL        (~(0x00) ^ LOG_SCOPE_NEON)

// define log scopes
extern const char* SCOPE_FILE;      // Davix file interface
extern const char* SCOPE_POSIX;     // Davix posix interface
extern const char* SCOPE_XML;       // XML info and parser output
extern const char* SCOPE_SSL;       // SSL and cert details
extern const char* SCOPE_HEADER;    // Request and respond headers
extern const char* SCOPE_BODY;      // HTTP bodies
extern const char* SCOPE_CHAIN;     // IO chains info
extern const char* SCOPE_CORE;      // Config and misc info
extern const char* SCOPE_GRID;      // Misc info from 3rd parties
extern const char* SCOPE_SOCKET;    // Socket info
extern const char* SCOPE_LOCKS;     // WebDAV locking info
extern const char* SCOPE_ALL;       // All of the above

/// set the davix log mask
/// everything that is not coverred by the mask is dropped
extern DAVIX_EXPORT void davix_set_log_level(int log_mask);

/// get current log mask
extern DAVIX_EXPORT int davix_get_log_level();

/// internal logger function
extern DAVIX_EXPORT void davix_logger(int log_mask, const char * msg, ...);

/// variadic version of internal logger function
extern DAVIX_EXPORT void davix_vlogger(int log_mask, const char* msg, va_list args);


/// @brief setup a log handler
///
/// redirect the davix log output to the function f_handler
/// setting up a log handler disable the std output log
///
/// @param fhandler : log handler callback
/// @param userdata : callback userdata
extern DAVIX_EXPORT void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata);

/// Compare command line tools --trace options with scopes and set mask
void davix_set_log_scope(const std::string & scope);

std::string davix_get_log_scope(int scope_mask);

void davix_set_log_debug(bool dbg);

bool davix_get_log_debug();    

/// Set debug level
void davix_set_trace_level(int trace_level);

/// Get debug level
int davix_get_trace_level();

DAVIX_C_DECL_END

#endif // DAVIX_LOGGER_H
