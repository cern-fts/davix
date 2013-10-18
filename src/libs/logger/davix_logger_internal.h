#ifndef DAVIX_LOGGER_INTERNAL_H
#define DAVIX_LOGGER_INTERNAL_H

#include <davix_types.h>
#include <logger/davix_logger.h>

DAVIX_C_DECL_BEGIN

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


#define davix_return_val_if_fail(X, Y) \
    do{ \
        if(!(X)){ \
            DAVIX_DEBUG("WARNING: davix assertion failed at %s:%s", __LINE__, __FILE__); \
            return Y; \
        } \
    }while(0)


DAVIX_C_DECL_END

#endif // DAVIX_LOGGER_INTERNAL_H
