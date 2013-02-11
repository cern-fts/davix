#include <config.h>
#include <logger/davix_logger.h>
#include <logger/logger.h>


static int internal_log_mask = DAVIX_LOG_CRITICAL;



void davix_set_log_level(int log_mask){
    internal_log_mask = log_mask;
}

int davix_get_log_level(){
    return internal_log_mask;
}

void davix_logger(int log_mask, const char * msg, ...){
    va_list va;
    va_start(va, msg);
    g_loggerv("DAVIX", G_LOG_LEVEL_MESSAGE, msg, va);
    va_end(va);
}
