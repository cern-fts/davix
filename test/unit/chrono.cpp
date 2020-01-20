#include <davix_internal_config.hpp>
#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include <ctime>
#include <cstring>

#undef A_LIB_NAMESPACE
#define A_LIB_NAMESPACE Davix

#include "libs/alibxx/alibxx.hpp"

using namespace std;


// instanciate and play with gates
TEST(ALibxx, ChronoTest){

    using namespace Davix::Chrono;

    TimePoint pp;
    ASSERT_EQ(0, pp.toTimestamp());

    pp = Clock(Clock::Monolitic).now();
    TimePoint copy_time = pp;
    ASSERT_EQ(pp, copy_time);
    copy_time = copy_time + Duration(5);
    ASSERT_NE(copy_time, pp);
    ASSERT_EQ(copy_time, pp + Duration(5));
    ASSERT_GE(copy_time, pp);
    ASSERT_GE(copy_time, pp + Duration(5));
    ASSERT_GT(copy_time, pp);
    ASSERT_LE(copy_time, pp + Duration(5));
    ASSERT_LE(copy_time, pp + Duration(10));
    ASSERT_LT(copy_time, pp + Duration(10));

    ASSERT_EQ(copy_time + Duration(5), pp + Duration(10));

    copy_time = pp;
    ASSERT_EQ(copy_time, pp + Duration(10) - Duration(20) + Duration(10));

    TimePoint dd;
    ASSERT_ANY_THROW(
                 Duration res = dd - pp;
                );

    Duration res = pp - dd;
    ASSERT_EQ( dd + res, pp);
}


