#include "test_datetime.h"
#include <ctime>
#include <datetime/datetime_utils.h>

void test_datetime_iso8601(){
    GError * tmp_err=NULL;
    time_t res, t = time(NULL);
    struct tm* tmp = gmtime(&t);
    char buff[2048];
    char buff2[2048];
    char buff3[2048];
    strftime(buff, 2048, "%Y-%m-%dT%H:%M:%S%z", tmp);
    printf("new iso8601 time %s  \n",buff);
    res= parse_iso8601date(buff, &tmp_err);
    assert_true_with_message(res == t && tmp_err==NULL, " must be the same instant %ld %ld, ctime : %s %s \n",res,t, ctime_r(&res,buff2), ctime_r(&t,buff3));

    res = parse_iso8601date("unknow invaldi time", &tmp_err);
    assert_true_with_message(res == -1 && tmp_err != NULL, " must be an error report \n");
    if(tmp_err){
        printf(" error : %s \n", tmp_err->message);
        g_clear_error(&tmp_err);
    }
}


TestSuite * datetime_suite (void)
{
        TestSuite *s2 = create_test_suite();
        // verbose test case /
        add_test(s2, test_datetime_iso8601);

        return s2;
 }


