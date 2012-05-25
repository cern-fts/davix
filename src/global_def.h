#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

#include <glibmm.h>

#include <iostream>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <logging/logger.h>

#include <davix.h>

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
    DAVIX_FILE
} RequestType;




#define DAVIX_GRID_NONE 0x00
#define DAVIX_GRID_SSL_CHECK 0x01           /**< grid_mode : disable the SSL ca checking, usefull for worker node configuration */
#define DAVIX_GRID_AUTO_VOMS 0x02           /**< grid mode : enable the detection of VOMS certificate, enable it if found */
#define DAVIX_GRID_ALL (~(0x00))            /**< enable all the grid mode */


#define DAVIX_BUFFER_SIZE 2048


}

#endif // GLOBAL_DEF_H
