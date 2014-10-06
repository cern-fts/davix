#ifndef alibxx_alibxx_LOGGER_HPP
#define alibxx_alibxx_LOGGER_HPP

#include <bitset>

namespace A_LIB_NAMESPACE {

typedef std::bitset<64> Channels;
typedef unsigned int LogLevel;



class Logger
{
public:
    static const LogLevel Critical;
    static const LogLevel Warning;
    static const LogLevel Verbose;
    static const LogLevel Debug;
    static const LogLevel Trace;


    static inline Channels allComponents(){
        Channels mask;
        mask.set();
        return mask;
    }

    Logger();

    virtual ~Logger();

    inline void setComponentMask(int index, bool flag){
        chnl.set(index, flag);
    }

    inline void setLogLevel(const LogLevel level){
        log_level = level;
    }

    inline LogLevel getLogLevel() const{
        return log_level;
    }

    inline bool componentEnabled(int index){
        return chnl[index];
    }


    void logStream(const std::ostream & stream);


#define alibxx_LOG(LOG_LEVEL,  COMPONENT_INT, MSG) if( LOG_LEVEL >= getLogLevel() && componentEnabled(COMPONENT_INT)){ std::ostringstream ss; ss << MSG

    void reset(const LogLevel ldefault = Warning, const Channels & mask_default= allComponents());

private:
    Channels chnl;
    LogLevel log_level;

};

} // namespace alibxx

#endif // alibxx_alibxx_LOGGER_HPP
