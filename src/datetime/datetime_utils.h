#ifndef DATETIME_UTILS_H
#define DATETIME_UTILS_H

#include <davix.h>
#include <string.h>
#include <time.h>

#define DATETIME_UTILS_PARSE_ERROR 1
#define DATETIME_UTILS_CONV_ERROR 2

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif

#endif // DATETIME_UTILS_H
