
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>
#include <gtest/gtest.h>

TEST(SessionTest, create_session){
    Davix_error * tmp_err=NULL;
    davix_sess_t ctxt = davix_context_new(&tmp_err);
    ASSERT_TRUE(ctxt != NULL);
    ASSERT_TRUE(tmp_err==NULL);
    davix_context_free(ctxt);
}

