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

#include <vector>
#include <cstdio>
#include <cstdarg>
#include <davix_internal.hpp>
#include <utils/davix_logger.hpp>
#include <utils/stringutils.hpp>

#ifdef HAVE_ATOMIC
#include <atomic>
static std::atomic<int> internal_log_level(0);
static std::atomic<int> internal_log_scope(DAVIX_LOG_SCOPE_ALL);
#else
#warning "Setting / getting davix loglevel is not thread-safe!"
int internal_log_level = 0;
int internal_log_scope = DAVIX_LOG_SCOPE_ALL;
#endif

const int BUFFER_SIZE =4096;
const char* prefix = "DAVIX";

static void (*_fhandler)(void* userdata, int mgs_level, const char* msg) = NULL;
static void* _log_handler_userdata = NULL;






namespace Davix{

const char* SCOPE_FILE =    "file";
const char* SCOPE_HTTP =    "http";
const char* SCOPE_POSIX =   "posix";
const char* SCOPE_XML =     "xml";
const char* SCOPE_SSL =     "ssl";
const char* SCOPE_HEADER =  "header";
const char* SCOPE_BODY =    "body";
const char* SCOPE_CHAIN =   "chain";
const char* SCOPE_CORE =    "core";
const char* SCOPE_GRID =    "grid";
const char* SCOPE_SOCKET =  "socket";
const char* SCOPE_LOCKS =   "locks";
const char* SCOPE_S3 =      "s3";
const char* SCOPE_SENSITIVE = "sensitive";
const char* SCOPE_ALL =     "all";


const std::string davix_log_scope[] = {
    SCOPE_FILE,
    SCOPE_HTTP,
    SCOPE_POSIX,
    SCOPE_XML,
    SCOPE_SSL,
    SCOPE_HEADER,
    SCOPE_BODY,
    SCOPE_CHAIN,
    SCOPE_CORE,
    SCOPE_GRID,
    SCOPE_SOCKET,
    SCOPE_LOCKS,
    SCOPE_S3,
    SCOPE_SENSITIVE,
    SCOPE_ALL
};

const int num_of_scopes = sizeof(davix_log_scope)/sizeof(*davix_log_scope);


int getLogLevel(){
    return internal_log_level;
}

void setLogLevel(int logLevel){
    internal_log_level = logLevel;
}

int getLogScope(){
    return internal_log_scope;
}

void setLogScope(int log_scope){
    internal_log_scope = log_scope;
}

void setLogScope(const std::string &scope){
    int mask=0;
    std::vector<std::string> vec_scopes;
    StrUtil::split( scope, ',', vec_scopes);

    for(std::vector<std::string>::iterator it = vec_scopes.begin(); it < vec_scopes.end(); ++it){
        for(int i = 0; i < num_of_scopes; ++i){
            if(StrUtil::compare_ncase((*it), davix_log_scope[i]) == 0){
                switch(i){
                    case 0:
                        mask |= DAVIX_LOG_FILE;
                        break;
                    case 1:
                        mask |= DAVIX_LOG_HTTP;
                        break;
                    case 2:
                        mask |= DAVIX_LOG_POSIX;
                        break;
                    case 3:
                        mask |= DAVIX_LOG_XML;
                        break;
                    case 4:
                        mask |= DAVIX_LOG_SSL;
                        break;
                    case 5:
                        mask |= DAVIX_LOG_HEADER;
                        break;
                    case 6:
                        mask |= DAVIX_LOG_BODY;
                        break;
                    case 7:
                        mask |= DAVIX_LOG_CHAIN;
                        break;
                    case 8:
                        mask |= DAVIX_LOG_CORE;
                        break;
                    case 9:
                        mask |= DAVIX_LOG_GRID;
                        break;
                    case 10:
                        mask |= DAVIX_LOG_SOCKET;
                        break;
                    case 11:
                        mask |= DAVIX_LOG_LOCKS;
                        break;
                    case 12:
                        mask |= DAVIX_LOG_S3;
                        break;
                  case 13:
                        mask |= DAVIX_LOG_SENSITIVE;
                        break;
                  case 14:
                        mask |= DAVIX_LOG_SCOPE_ALL;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    setLogScope(mask);
}



void logStr(int scope, int log_level, const std::string & str){
    if(_fhandler){
        _fhandler(_log_handler_userdata, log_level, str.c_str());
    }else{
        if(scope & DAVIX_LOG_HEADER){ // log header, we do not want headers to be prefixed
            fmt::print(stderr, "{}\n", str);
        }else{  // davix logs
            fmt::print(stderr,"{}({}): {}\n", prefix, getScopeName(scope), str);
        }
    }
}



std::string getScopeName(int scope_mask){
    std::string scope_name;
    switch(scope_mask){
        case DAVIX_LOG_FILE:
            scope_name = SCOPE_FILE;
            break;
        case DAVIX_LOG_HTTP:
            scope_name = SCOPE_HTTP;
            break;
        case DAVIX_LOG_POSIX:
            scope_name = SCOPE_POSIX;
            break;
        case DAVIX_LOG_XML:
            scope_name = SCOPE_XML;
            break;
        case DAVIX_LOG_SSL:
            scope_name = SCOPE_SSL;
            break;
        case DAVIX_LOG_HEADER:
            scope_name = SCOPE_HEADER;
            break;
        case DAVIX_LOG_BODY:
            scope_name = SCOPE_BODY;
            break;
        case DAVIX_LOG_CHAIN:
            scope_name = SCOPE_CHAIN;
            break;
        case DAVIX_LOG_CORE:
            scope_name = SCOPE_CORE;
            break;
        case DAVIX_LOG_GRID:
            scope_name = SCOPE_GRID;
            break;
        case DAVIX_LOG_SOCKET:
            scope_name = SCOPE_SOCKET;
            break;
        case DAVIX_LOG_LOCKS:
            scope_name = SCOPE_LOCKS;
            break;
        case DAVIX_LOG_S3:
            scope_name = SCOPE_S3;
            break;
        default:
            scope_name = "Unknown";
            break;
    }
    return scope_name;
}






} // Davix



//
// Old legacy API
//


extern "C" void davix_set_log_level(int log_mask){
    internal_log_level = log_mask;
}

extern "C" int davix_get_log_level(){
    return internal_log_level;
}


extern "C" int davix_get_log_scope(){
    return internal_log_scope;
}


extern "C" void davix_vlogger2(int log_mask, int log_level, const char* msg, va_list ap){
    char buffer[BUFFER_SIZE];

    vsnprintf(buffer, BUFFER_SIZE-1, msg, ap);
    buffer[BUFFER_SIZE-1] ='\0';
    Davix::logStr(log_mask, log_level, buffer);
}

extern "C" void davix_vlogger(int log_mask, const char* msg, va_list va){
    davix_vlogger2(log_mask, DAVIX_LOG_DEBUG, msg, va);
}



extern "C" void davix_logger(int log_mask, const char * msg, ...){
    va_list va;
    va_start(va, msg);
    davix_vlogger(log_mask, msg, va);
    va_end(va);
}


extern "C"  void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata){
    _fhandler = fhandler;
    _log_handler_userdata = userdata;
}
