#pragma once

#include <ctime>
#include <ostream>

#include "../base_types.hpp"
#include "../operator.hpp"
#include "../alibxx.hpp"

#include "chrono_exception.hpp"

namespace A_LIB_NAMESPACE{

namespace Chrono {

class Clock;
class Duration;
class TimePoint;

/// @class TimePoint
///  represent a time position
class TimePoint : public Operator::Equality<TimePoint, TimePoint>,
                  public Operator::Compare<TimePoint, TimePoint>,
                  public Operator::Subtractable<TimePoint, Duration>,
                  public Operator::Addable<TimePoint, Duration>
{
public:

    TimePoint();
    ~TimePoint();

    bool operator==(const TimePoint & other) const;
    bool operator<(const TimePoint & other) const;

    // add time duration
    TimePoint & operator+=(const Duration & duration);
    TimePoint & operator-=(const Duration & duration);


    // extract duration from TimePoint
    // if time1 - time2 with time1 < time2 => throw ChronoException
    Duration operator-(const TimePoint & other) const;

    Type::UInt64 toTimestamp() const;

    bool isValid() const;



private:
    struct timespec t;
    friend class Clock;
    friend class Duration;
    friend std::ostream& operator<<(std::ostream& os, const TimePoint & t);
};

class Duration{
public:
    Duration();
    Duration(Type::UInt64 seconds);
    ~Duration();

    static Duration milliseconds(Type::UInt64 v);

    /// return duration value in seconds
    Type::UInt64 toTimeValue() const;

    /// return duration value in milliseconds
    Type::UInt64 toMilliseconds() const;

private:
    struct timespec t;
    friend class Clock;
    friend class TimePoint;
    friend std::ostream& operator<<(std::ostream& os, const Duration & d);
};




class Clock{
public:
    enum Type{
        Monolitic,
        RealTime
    };
    enum Precision{
        Second,
        MilliSecond,
        MicroSecond,
        NanoSecond
    };

    Clock(Type clock_type, Precision tick = Second);
    ~Clock();

    TimePoint now() const;

private:
    Type _type;
    Precision _precision;
};



std::ostream& operator<<(std::ostream& os, const Duration & d);
std::ostream& operator<<(std::ostream& os, const TimePoint & d);


} // namespace Chrono



} // A_LIB_NAMESPACE
