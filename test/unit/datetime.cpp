#include <gtest/gtest.h>
#include <ctime>
#include <datetime/datetime_utils.hpp>

TEST(DateTimeTest, testConvert){
    time_t res, t = time(NULL);
    struct tm* tmp = gmtime(&t);
    char buff[2048];
    strftime(buff, 2048, "%Y-%m-%dT%H:%M:%SZ", tmp);
    printf("new iso8601 time %s  \n",buff);
    res= parse_iso8601date(buff);
    ASSERT_EQ(t,res);

    res = parse_iso8601date("unknow invalid time");
    ASSERT_EQ(-1, res);
}



