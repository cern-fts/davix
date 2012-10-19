#include <gtest/gtest.h>
#include <ctime>
#include <datetime/datetime_utils.h>

TEST(DateTimeTest, testConvert){
    GError * tmp_err=NULL;
    time_t res, t = time(NULL);
    struct tm* tmp = gmtime(&t);
    char buff[2048];
    strftime(buff, 2048, "%Y-%m-%dT%H:%M:%S%z", tmp);
    printf("new iso8601 time %s  \n",buff);
    res= parse_iso8601date(buff, &tmp_err);
    ASSERT_EQ(t,res);
    ASSERT_TRUE(tmp_err==NULL);

    res = parse_iso8601date("unknow invalid time", &tmp_err);
    ASSERT_EQ(-1, res);
    ASSERT_TRUE(tmp_err != NULL);
    if(tmp_err){
        printf(" error : %s \n", tmp_err->message);
        g_clear_error(&tmp_err);
    }
}



