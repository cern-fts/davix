
#include <ctime>
#include "libs/datetime/datetime_utils.hpp"
#include <davix.hpp>
#include <gtest/gtest.h>

TEST(SessionTest, create_session){
    Davix_error * tmp_err=NULL;
    Davix::Context c;
    Davix::Context* ctxt = new Davix::Context();
    ASSERT_TRUE(ctxt != NULL);
    ASSERT_TRUE(tmp_err==NULL);
    delete ctxt;
}

