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





///////////////////////////////
//////////////////////////////


Clock::Clock(Type clock_type, Precision tick) : _type(clock_type), _precision(tick){

}

Clock::~Clock(){

}

TimePoint Clock::now() const{
    TimePoint res;
    clockid_t t;
    switch(_type){
        case Monolitic:
            t = CLOCK_MONOTONIC;
            break;
        default:
            t = CLOCK_REALTIME;
            break;
    }
    clock_gettime(t, &(res.t));
    return res;
}


std::ostream& operator<<(std::ostream& os, const Duration & d){
    os << d.t.tv_sec << "s";
    return os;
}

std::ostream& operator<<(std::ostream& os, const TimePoint & t){
    os << t;
    return os;
}



} // namespace Chrono


}
