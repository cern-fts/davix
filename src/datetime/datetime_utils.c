#define _GNU_SOURCE

#include "datetime_utils.h"



time_t parse_http_date(const char* http_date, GError** err){
    static const char rfc1123[] = "%a, %d %b %Y %H:%M:%S GMT";
    struct tm tm;
    time_t mtime;
    memset(&tm,0, sizeof(struct tm));

    const char * p = strptime(http_date, rfc1123, &tm);
    if ( p == NULL || *p != '\0'){
        g_set_error(err, g_quark_from_string("parse_http_date"), DATETIME_UTILS_PARSE_ERROR, "HTTP time value parsing error %s", http_date);
        return -1;
    }
    if( (mtime = mktime(&tm)) == -1){
        g_set_error(err, g_quark_from_string("parse_http_date"), DATETIME_UTILS_CONV_ERROR, "HTTP time convertion error %s", http_date);
        return -1;
    }
    return mtime;

}


time_t parse_iso8601date(const char* iso_date, GError** err){

    // old part of code, should be corrected in the futur, EL 5 SL 5 compat
    GTimeVal timval;
    time_t res = -1;
    if( g_time_val_from_iso8601 (iso_date, &timval) == TRUE){
        res = (time_t) timval.tv_sec;
    }else
       g_set_error(err, g_quark_from_string("parse_iso8601date"), DATETIME_UTILS_PARSE_ERROR, "iso8601 parsing error");

    return res;

}
