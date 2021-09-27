#include <gtest/gtest.h>
#include <ctime>
#include "libs/datetime/datetime_utils.hpp"

TEST(DateTimeTest, testConvert){
    time_t res, t = time(NULL);
    struct tm* tmp = gmtime(&t);
    char buff[2048];
    strftime(buff, 2048, "%Y-%m-%dT%H:%M:%SZ", tmp);
    printf("new iso8601 time %s  \n",buff);
    res= parse_iso8601date(buff);
    ASSERT_EQ(t,res);

    res = parse_iso8601date("unknown invalid time");
    ASSERT_EQ(-1, res);
}

TEST(DateTimeTest, Iso8601Test1) {
    // was failing on FreeBSD
    std::string testcase("2018-10-02T17:19:00Z");
    ASSERT_EQ(parse_iso8601date(testcase.c_str()), 1538500740);
}


