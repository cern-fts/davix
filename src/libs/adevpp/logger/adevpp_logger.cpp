#include "adevpp_logger.hpp"

namespace Adevpp {

const LogLevel Logger::Critical=0;
const LogLevel Logger::Warning=0x01;
const LogLevel Logger::Verbose=0x10;
const LogLevel Logger::Debug=0x100;
const LogLevel Logger::Trace=0x1000;


Logger::Logger()
{
    reset();
}

Logger::~Logger(){

}

void Logger::reset(const LogLevel ldefault = Warning, const CmptMask & mask_default= CmptMask()){
    cmpts = CmptMask;
    log_level = ldefault;
}


} // namespace Adevpp
