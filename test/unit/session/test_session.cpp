
#include <ctime>
#include <datetime/datetime_utils.h>
#include <davix.h>
#include <gtest/gtest.h>

TEST(SessionTest, create_session){
    GError * tmp_err=NULL;
    davix_sess_t ctxt = davix_session_new(&tmp_err);
    ASSERT_TRUE(ctxt != NULL);
    ASSERT_TRUE(tmp_err==NULL);
    davix_session_free(ctxt);
}

