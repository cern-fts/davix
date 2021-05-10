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


#ifndef DAVIX_LOGGER_C_H
#define DAVIX_LOGGER_C_H


// log level
#define DAVIX_LOG_CRITICAL  1
#define DAVIX_LOG_WARNING   2
#define DAVIX_LOG_VERBOSE   3
#define DAVIX_LOG_DEBUG     4
#define DAVIX_LOG_TRACE     5
#define DAVIX_LOG_ALL       6

// log scope
#define DAVIX_LOG_FILE       (1<<0)
#define DAVIX_LOG_POSIX      (1<<1)
#define DAVIX_LOG_XML        (1<<2)
#define DAVIX_LOG_SSL        (1<<3)
#define DAVIX_LOG_HEADER     (1<<4)
#define DAVIX_LOG_BODY       (1<<5)
#define DAVIX_LOG_CHAIN      (1<<6)
#define DAVIX_LOG_CORE       (1<<7)
#define DAVIX_LOG_GRID       (1<<8)
#define DAVIX_LOG_SOCKET     (1<<9)
#define DAVIX_LOG_LOCKS      (1<<10)
#define DAVIX_LOG_HTTP       (1<<11)
#define DAVIX_LOG_S3         (1<<12)
#define DAVIX_LOG_SENSITIVE  (1<<13)
#define DAVIX_LOG_SCOPE_NEON (1<<29)
#define DAVIX_LOG_SCOPE_ALL        (~(0) ^ DAVIX_LOG_BODY)

/* set the davix log mask */
/* everything that is not covered by the mask is dropped */
void davix_set_log_level(int log_mask);

/* get current log mask */
int davix_get_log_level();


void davix_vlogger2(int log_mask, int log_level, const char* msg, va_list args);


int davix_get_log_scope();

/* @brief setup a log handler

   redirect the davix log output to the function f_handler
   setting up a log handler disable the std output log

   @param fhandler : log handler callback
   @param userdata : callback userdata */
void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata);

#endif /* DAVIX_LOGGER_C_H */
