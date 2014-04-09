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

#include <davix_internal.hpp>
#include <cstdio>
#include <cstdarg>
#include <utils/davix_logger.hpp>


const int BUFFER_SIZE =4096;
const char* prefix = "DAVIX: ";


static int internal_log_mask = DAVIX_LOG_CRITICAL;
static void (*_fhandler)(void* userdata, int mgs_level, const char* msg) = NULL;
static void* _log_handler_userdata = NULL;

extern "C" void davix_set_log_level(int log_mask){
    internal_log_mask = log_mask;
}

extern "C" int davix_get_log_level(){
    return internal_log_mask;
}

static void internal_log_handler(int log_mask, const char * msg,  va_list ap){
    char buffer[BUFFER_SIZE];

    vsnprintf(buffer, BUFFER_SIZE-1, msg, ap);
    buffer[BUFFER_SIZE-1] ='\0';
    if(_fhandler){
        _fhandler(_log_handler_userdata, log_mask, buffer);
    }else{
        fprintf(stdout, "%s%s\n",prefix, buffer);
    }
}

extern "C" void davix_logger(int log_mask, const char * msg, ...){
    va_list va;
    va_start(va, msg);
    internal_log_handler(log_mask, msg, va);
    va_end(va);
}




extern "C"  void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata){
    _fhandler = fhandler;
    _log_handler_userdata = userdata;
}
