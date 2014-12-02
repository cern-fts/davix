#include <davix_internal_config.hpp>
#include "timepoint.hpp"


namespace A_LIB_NAMESPACE{


namespace Chrono {

TimePoint::TimePoint() : t()
{
    t.tv_nsec =0;
    t.tv_sec =0;
}

TimePoint::~TimePoint(){

}


bool TimePoint::operator ==(const TimePoint & other) const{
    return ( (t.tv_nsec == other.t.tv_nsec) && (t.tv_sec == other.t.tv_sec));
}


bool TimePoint::operator<(const TimePoint & other) const{
    return ( (t.tv_sec < other.t.tv_sec)
             || (( t.tv_sec == other.t.tv_sec) && (t.tv_nsec < other.t.tv_nsec)));
}

TimePoint & TimePoint::operator+=(const Duration & duration){
    t.tv_sec += duration.t.tv_sec;
    t.tv_nsec += duration.t.tv_nsec;
    return *this;
}

TimePoint & TimePoint::operator-=(const Duration & duration){
    t.tv_sec -= duration.t.tv_sec;
    t.tv_nsec -= duration.t.tv_nsec;
    return *this;
}


Duration TimePoint::operator-(const TimePoint & timepoint) const{
    Duration d;
    if(*this < timepoint){
        throw ChronoException("Negative duration between two TimePoint");
    }
    d.t.tv_sec = (t.tv_sec - timepoint.t.tv_sec);
    d.t.tv_nsec = (t.tv_nsec - timepoint.t.tv_nsec);
    return d;
}

Type::UInt64 TimePoint::toTimestamp() const{
    return static_cast<Type::UInt64>(t.tv_sec);
}


bool TimePoint::isValid(){
    return (t.tv_sec != 0 && t.tv_nsec != 0);
}



////////////////////////////////
////////////////////////////////

Duration::Duration(){
    t.tv_sec =0;
    t.tv_nsec =0;
}

Duration::Duration(Type::UInt64 seconds){
    t.tv_nsec =0;
    t.tv_sec = seconds;
}

Duration::~Duration(){

}

Type::UInt64 Duration::toTimeValue() const{
    return t.tv_sec;
}





///////////////////////////////
//////////////////////////////


static void get_time(Clock::Type clock_type, struct timespec & time_value){
#ifdef HAVE_CLOCK_GETTIME
    clockid_t t;
    switch(clock_type){
        case Clock::Monolitic:
            t = CLOCK_MONOTONIC;
            break;
        default:
            t = CLOCK_REALTIME;
            break;
    }
    clock_gettime(t, &time_value);
#elif HAVE_GETTIMEOFDAY
    // TODO: gettimeofday is vulnerable to time jump
    // need an OSX specific implementation using Mach micro kernel API
    struct timeval now;
    (void) gettimeofday(&now, NULL);
    time_value.tv_sec  = now.tv_sec;
    time_value.tv_nsec = now.tv_usec * 1000;
#else
#error "No gettimeofday nor clock_gettime: No time support"
#endif

}


Clock::Clock(Type clock_type, Precision tick) : _type(clock_type), _precision(tick){
    (void) _precision;
}

Clock::~Clock(){

}

TimePoint Clock::now() const{
    TimePoint res;
    get_time(_type, res.t);
    return res;
}


std::ostream& operator<<(std::ostream& os, const Duration & d){
    os << d.t.tv_sec;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TimePoint & t){
    os << t.t.tv_sec;
    return os;
}



} // namespace Chrono


}
