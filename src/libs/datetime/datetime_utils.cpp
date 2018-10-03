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

#include <davix_internal_config.hpp>
#include "datetime_utils.hpp"

time_t parse_http_date(const char* http_date){
        // TODO : possible issue with timezone != GMT,

    static const char rfc1123[] = "%a, %d %b %Y %H:%M:%S GMT";
    struct tm tm;
    memset(&tm,0, sizeof(struct tm));

    const char * p = strptime(http_date, rfc1123, &tm);
    if ( p == NULL || *p != '\0'){
        return -1;
    }
    return timegm(&tm);

}


time_t parse_iso8601date(const char* iso_date){

    // TODO : refactor to a regex parser.. ISO8601 b******* is impossible to parse correctly with strptime
    struct tm tm_time;
    char* p, *end_p;
    memset(&tm_time,0, sizeof(struct tm ));
    if(  ( (p = strptime(iso_date, "%Y-%m-%dT%H:%M:%SZ", &tm_time) ) == NULL || *p != '\0')
         && ( (p = strptime(iso_date, "%Y-%m-%dT%H:%M:%S", &tm_time) ) == NULL || *p != '.'
              ||  *(iso_date + strlen(iso_date) -1) != 'Z' )){
        if( (p = strptime(iso_date, "%Y-%m-%dT%H:%M:%S", &tm_time) ) != NULL
                && (*p == '+' || *p == '-') ){
                struct tm tm_time_offset;
                memset(&tm_time_offset,0, sizeof(struct tm ));
                if( ( ( end_p= strptime(p+1,  "%H:%M", &tm_time_offset)) == NULL
                      || *end_p != '\0'
                     )
                    && ( (end_p =strptime(p+1,  "%H%M", &tm_time_offset)) == NULL
                         || *end_p != '\0'
                       )
                   )
                    return -1;
                if( *p =='+'){
                    tm_time.tm_hour +=  tm_time_offset.tm_hour;
                    tm_time.tm_min +=  tm_time_offset.tm_min;
                 } else{
                    tm_time.tm_hour -=  tm_time_offset.tm_hour;
                    tm_time.tm_min -=  tm_time_offset.tm_min;
                 }
        }else{
            return -1;
        }
    }


    return timegm(&tm_time);
}

time_t parse_standard_date(const char* http_date){
    if(strchr(http_date, ',') != NULL){ // detect if rfc date or iso8601 date
        return parse_http_date(http_date);
    }else{
         return parse_iso8601date(http_date);
    }
}

std::string Davix::current_time(const std::string format) {
    return Davix::time_as_string(time(NULL), format);
}

std::string Davix::time_as_string(const time_t t, const std::string format) {
    struct tm utc;
    char date[255];

    date[254] = '\0';
#ifdef HAVE_GMTIME_R
    gmtime_r(&t, &utc);
#else
    struct tm* p_utc = gmtime(&t);
    memcpy(&utc_current, p_utc, sizeof(struct tm));
#endif
    strftime(date, 254, format.c_str(), &utc);
    return std::string(date);
}

