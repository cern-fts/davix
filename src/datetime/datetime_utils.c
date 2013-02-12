#include <config.h>
#include "datetime_utils.h"



time_t parse_http_date(const char* http_date){
    static const char rfc1123[] = "%a, %d %b %Y %H:%M:%S GMT";
    struct tm tm;
    time_t mtime;
    memset(&tm,0, sizeof(struct tm));

    const char * p = strptime(http_date, rfc1123, &tm);
    if ( p == NULL || *p != '\0'){
        return -1;
    }
    if( (mtime = mktime(&tm)) == -1){
        return -1;
    }
    return mtime;

}


time_t parse_iso8601date(const char* iso_date){

    // old part of code, should be corrected in the futur, EL 5 SL 5 compat
    GTimeVal timval;
    time_t res = -1;
    if( g_time_val_from_iso8601 (iso_date, &timval) == TRUE){
        res = (time_t) timval.tv_sec;
    }
    return res;

}

time_t parse_standard_date(const char* http_date){
    if(strchr(http_date, ',') != NULL){ // detect if rfc date or iso8601 date
        return parse_http_date(http_date);
    }else{
         return parse_iso8601date(http_date);
    }
}
