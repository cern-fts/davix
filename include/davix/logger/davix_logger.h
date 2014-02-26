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


#ifndef DAVIX_LOGGER_H
#define DAVIX_LOGGER_H

#include <davix_types.h>

DAVIX_C_DECL_BEGIN

#define DAVIX_LOG_CRITICAL  0x00
#define DAVIX_LOG_WARNING   0x01
#define DAVIX_LOG_VERBOSE   0x02
#define DAVIX_LOG_DEBUG     0x04
#define DAVIX_LOG_TRACE     0x08
#define DAVIX_LOG_ALL       (~(0x00))


/// set the davix log mask
/// everything that is not coverred by the mask is dropped
extern DAVIX_EXPORT void davix_set_log_level(int log_mask);

/// get current log mask
extern DAVIX_EXPORT int davix_get_log_level();

extern DAVIX_EXPORT void davix_logger(int log_mask, const char * msg, ...);

/// @brief setup a log handler
///
/// redirect the davix log output to the function f_handler
/// setting up a log handler disable the std output log
///
/// @param fhandler : log handler callback
/// @param userdata : callback userdata
extern DAVIX_EXPORT void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata);

DAVIX_C_DECL_END

#endif // DAVIX_LOGGER_H
