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

#ifndef DAVIX_LOGGER_INTERNAL_HPP
#define DAVIX_LOGGER_INTERNAL_HPP



#include "davix_internal_config.hpp"
#include "../libs/alibxx/str/format.hpp"

#include <utils/davix_logger.hpp>

namespace Davix{


// log a string message to the davix logger
void logStr(int scope, int log_level, const std::string & str);

// Simple logger to trace in / out of function scope
//
class ScopeLogger{
public:
    ScopeLogger(int scopep, const char* msgp) : scope(0), msg(NULL){
        if( getLogLevel() >= DAVIX_LOG_TRACE && getLogScope() & scopep){
            msg = msgp;
            scope = scopep;
            logStr(scope, davix_get_log_level(), ::Davix::fmt::format(" -> {}",msg));
        }
    }

    ~ScopeLogger(){
        if(msg){
            logStr(scope, davix_get_log_level(), ::Davix::fmt::format(" <- {}",msg));
        }
    }

private:
    int scope;
    const char* msg;
};



#define DAVIX_SLOG(lvl, scope, msg, ...) \
    do{ \
    if( (::Davix::getLogScope() & scope) && (::Davix::getLogLevel() >= lvl) ){ \
        ::Davix::logStr(scope, lvl, ::Davix::fmt::format(msg, ##__VA_ARGS__)); } \
    }while(0)


#define DAVIX_SCOPE_TRACE(scope, id) \
    ::Davix::ScopeLogger id(scope, __func__)



} // Davix

#endif // DAVIX_LOGGER_INTERNAL_HPP
