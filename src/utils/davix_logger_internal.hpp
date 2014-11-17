#ifndef DAVIX_LOGGER_INTERNAL_HPP
#define DAVIX_LOGGER_INTERNAL_HPP

#include <utils/davix_types.hpp>
#include <utils/davix_logger.hpp>

DAVIX_C_DECL_BEGIN

#define davix_return_val_if_fail(X, Y) \
    do{ \
        if(!(X)){ \
            DAVIX_LOG(DAVIX_LOG_ALL, LOG_ALL, "WARNING: davix assertion failed at %s:%s", __LINE__, __FILE__); \
            return Y; \
        } \
    }while(0)


#define DAVIX_LOG(lvl, scope, msg, ...) \
    do{ \
    if( (davix_get_log_level() & scope) && (davix_get_trace_level() >= lvl) ){ \
        davix_logger(scope, msg, ##__VA_ARGS__); } \
    }while(0)

DAVIX_C_DECL_END

#endif // DAVIX_LOGGER_INTERNAL_HPP
