#include "logger.h"

/**
  every that does not match & with this filter is dropped
 */
GLogLevelFlags current_filter=G_LOG_LEVEL_MASK ;


void g_logger(const gchar *log_domain, GLogLevelFlags log_level,
              const gchar *format, ...){
    if( log_level & G_LOG_LEVEL_MASK){
        va_list va;
        va_start(va, format);
        g_logv(log_domain,
                 log_level,
                 format,
                 va);
        va_end(va);
    };
}

void g_loggerv(const gchar *log_domain, GLogLevelFlags log_level,
              const gchar *format, va_list va){
    if( log_level & G_LOG_LEVEL_MASK){
        g_logv(log_domain,
                 log_level,
                 format,
                 va);
    }
}

void g_logger_set_globalfilter(GLogLevelFlags flags){
    current_filter = flags;
}


GLogLevelFlags g_logger_get_globalfilter(){
    return current_filter;
}
