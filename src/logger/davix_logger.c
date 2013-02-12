#include <config.h>
#include <logger/davix_logger.h>
#include <stdarg.h>

const int BUFFER_SIZE =4096;
const char* prefix = "DAVIX: ";


static int internal_log_mask = DAVIX_LOG_CRITICAL;
static void (*_fhandler)(void* userdata, int mgs_level, const char* msg) = NULL;
static void* _log_handler_userdata = NULL;

void davix_set_log_level(int log_mask){
    internal_log_mask = log_mask;
}

int davix_get_log_level(){
    return internal_log_mask;
}

static void internal_log_handler(int log_mask, const char * msg,  va_list ap){
    char buffer[BUFFER_SIZE];

    vsnprintf(buffer, BUFFER_SIZE-1, msg, ap);
    buffer[BUFFER_SIZE-1] ='\0';
    if(_fhandler){
        _fhandler(_log_handler_userdata, log_mask, buffer);
    }else{
        fprintf(stdout, "%s%s\n",prefix, buffer);
    }
}

void davix_logger(int log_mask, const char * msg, ...){
    va_list va;
    va_start(va, msg);
    internal_log_handler(log_mask, msg, va);
    va_end(va);
}




void davix_set_log_handler( void (*fhandler)(void* userdata, int mgs_level, const char* msg), void* userdata){
    _fhandler = fhandler;
    _log_handler_userdata = userdata;
}
