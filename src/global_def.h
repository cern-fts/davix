#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include <iostream>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <logging/logger.h>

namespace Davix{

inline void davix_log_debug(const gchar *format,
                            ...){
    va_list va;
    va_start(va, format);
    g_loggerv("DAVIX", G_LOG_LEVEL_DEBUG ,
              format, va);
    va_end(va);
}

inline void davix_log_warning(const gchar *format,
                            ...){
    va_list va;
    va_start(va, format);
    g_loggerv("DAVIX", G_LOG_LEVEL_WARNING ,
              format, va);
    va_end(va);
}

typedef enum _REQUEST_TYPE{
    HTTP,
    HTTPS,
} RequestType;



#define DAVIX_GRID_NONE 0x00
#define DAVIX_GRID_SSL_CHECK 0x01           /**< grid_mode : disable the SSL ca checking, usefull for worker node configuration */
#define DAVIX_GRID_AUTO_CREDENTIAL 0x02     /**< grid mode : enable the load of the credential in the default globus location in a transparent way */
#define DAVIX_GRID_ALL (~(0x00))            /**< enable all the grid mode */

}

#endif // GLOBAL_DEF_H
