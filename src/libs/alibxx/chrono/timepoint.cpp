#include <davix_internal_config.hpp>
#include "timepoint.hpp"
#include <sys/time.h>

#if __cplusplus >= 201103L
#include <chrono>
#else
#include <sys/syscall.h>
#endif

static void gettimeofday_timespec(struct timespec *time_value) {
    struct timeval now;
    (void) gettimeofday(&now, NULL);
    time_value->tv_sec  = now.tv_sec;
    time_value->tv_nsec = now.tv_usec * 1000;
}

// also for use inside libneon
extern "C" {

// we should get rid of ifdefs as soon as we have C++11 on all platforms we care about
void davix_get_monotonic_time(struct timespec *time_value){
#if __cplusplus >= 201103L
    using namespace std;
    auto now = chrono::steady_clock::now().time_since_epoch();
    chrono::seconds sec = chrono::duration_cast<chrono::seconds>(now);

    time_value->tv_sec  = sec.count();
    time_value->tv_nsec = chrono::duration_cast<chrono::nanoseconds>(now - sec).count();
#else
#warning "Using clock_gettime"
    // todo: use the glibc wrapper for clock_gettime once it's available
    // on all the platforms we care about, instead of issuing a syscall
    // directly like barbarians
    //
    // linking with -lrt is NOT an option because the runtime dependency
    // is passed on to ROOT, which often needs a static build of davix
    int ret = syscall(SYS_clock_gettime, CLOCK_MONOTONIC, time_value);
    if(ret < 0) {
        gettimeofday_timespec(time_value);
    }
#endif
}
}

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


bool TimePoint::isValid() const {
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

Duration Duration::milliseconds(Type::UInt64 v) {
    Duration d;
    d.t.tv_sec = v / 1000;
    d.t.tv_nsec = (v % 1000) * 1000;
    return d;
}

Type::UInt64 Duration::toTimeValue() const{
    return t.tv_sec;
}

Type::UInt64 Duration::toMilliseconds() const {
    return (t.tv_sec * 1000) + (t.tv_nsec / 1000000);
}

///////////////////////////////
//////////////////////////////


static void get_time(Clock::Type clock_type, struct timespec & time_value) {
    switch(clock_type) {
        case Clock::Monolitic:
            davix_get_monotonic_time(&time_value);
            break;
        default:
            gettimeofday_timespec(&time_value);
            break;
    }
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
