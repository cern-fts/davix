#ifndef DAVIX_LOGGER_H
#define DAVIX_LOGGER_H

#include <davix_types.h>

DAVIX_C_DECL_BEGIN

#define DAVIX_LOG_CRITICAL  0x00
#define DAVIX_LOG_WARNING   0x01
#define DAVIX_LOG_VERBOSE   0x02
#define DAVIX_LOG_DEBUG     0x04
#define DAVIX_LOG_TRACE     0x08
#define DAVIX_LOG_ALL       0xFF


/// set the davix log mask
/// everything that is not coverred by the mask is dropped
void davix_set_log_level(int log_mask);

/// get current log mask
int davix_get_log_level();

void davix_logger(int log_mask, const char * msg, ...);

#define DAVIX_LOG(X, msg, ...) \
    do{ \
    if(X & davix_get_log_level()){ davix_logger(X, msg, ##__VA_ARGS__); } \
    }while(0)

#define DAVIX_DEBUG(msg, ...) \
    do{ \
    if(0x04 & davix_get_log_level() ){ davix_logger(0x04, msg, ##__VA_ARGS__); } \
    }while(0)

#define DAVIX_TRACE(msg, ...) \
    do{ \
    if(0x08 & davix_get_log_level()){ davix_logger(0x08, msg, ##__VA_ARGS__); } \
    }while(0)

DAVIX_C_DECL_END

#endif // DAVIX_LOGGER_H
