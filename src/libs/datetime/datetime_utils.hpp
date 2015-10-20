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

#ifndef DATETIME_UTILS_HPP
#define DATETIME_UTILS_HPP

#include <string.h>
#include <time.h>
#include <string>

#define DATETIME_UTILS_PARSE_ERROR 1
#define DATETIME_UTILS_CONV_ERROR 2


/**
  parse an http standard date to a posix time
  @param http_date http string of the date
  @return posix time or -1 if error occures
*/
time_t parse_http_date(const char* http_date);

/**
  parse a iso 8601 date to a posix time
  @param iso8601_date http string of the date
  @param err : Gerror error report system
  @return posix time or -1 if error occures and err is set
*/
time_t parse_iso8601date(const char* http_date);

/**
  parse both of iso 8601 and http standard date to a posix time
  @param http or iso8601 date
  @return posix time or -1 if error occures
*/
time_t parse_standard_date(const char* http_date);

namespace Davix {

/**
  give current time as string
  @param the format string to pass to strftime
  @return the current time using the requested formatting
*/
std::string current_time(const std::string format);

/**
  format time_t to string
  @param the time
  @param the format string to pass to strftime
  @return a string using the requested formatting
*/
std::string time_as_string(const time_t t, const std::string format);

}

#endif // DATETIME_UTILS_HPP
