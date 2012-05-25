#include "test_session.h"
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>

void test_session(){
    GError * tmp_err=NULL;
    davix_sess_t ctxt = davix_session_new(&tmp_err);
    assert_true_with_message(ctxt != NULL && tmp_err==NULL, " must be a valid context");
    davix_session_free(ctxt);
}


TestSuite * session_suite (void)
{
        TestSuite *s2 = create_test_suite();
        // verbose test case /
        add_test(s2, test_session);

        return s2;
 }
