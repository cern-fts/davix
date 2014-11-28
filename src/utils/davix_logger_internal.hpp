#ifndef DAVIX_LOGGER_INTERNAL_HPP
#define DAVIX_LOGGER_INTERNAL_HPP



#include "davix_internal_config.hpp"
#include "../libs/alibxx/str/format.hpp"

#include <utils/davix_logger.hpp>

namespace Davix{


// log a string message to the davix logger
void logStr(int scope, int log_level, const std::string & str);

// Simple logger to trace in / out of function scope
//
class ScopeLogger{
public:
    ScopeLogger(int scopep, const char* msgp) : scope(0), msg(NULL){
        if( davix_get_log_level() >= DAVIX_LOG_TRACE){
            msg = msgp;
            scope = scopep;
            logStr(scope, davix_get_log_level(), ::Davix::fmt::format(" -> {}",msg));
        }
    }

    ~ScopeLogger(){
        if(msg){
            logStr(scope, davix_get_log_level(), ::Davix::fmt::format(" <- {}",msg));
        }
    }

private:
    int scope;
    const char* msg;
};



#define DAVIX_SLOG(lvl, scope, msg, ...) \
    do{ \
    if( (davix_get_log_level() & scope) && (davix_get_trace_level() >= lvl) ){ \
        ::Davix::logStr(scope, lvl, ::Davix::fmt::format(msg, ##__VA_ARGS__)); } \
    }while(0)


#define DAVIX_SCOPE_TRACE(scope, id) \
    ::Davix::ScopeLogger id(scope, __func__)



} // Davix

#endif // DAVIX_LOGGER_INTERNAL_HPP
