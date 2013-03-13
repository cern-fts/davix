#include "test_httprequest.hpp"
#include <gtest/gtest.h>
#include <davix.hpp>
#include <request/httpcachetoken_internal.hpp>

using namespace Davix;

TEST(testHttpCacheToken, testCreate){
    davix_set_log_level(DAVIX_LOG_ALL);

    Uri u("http://higgs.boson/is/watchingus");
    HttpCacheToken t;
    HttpCacheToken t2(t);
    HttpCacheTokenAccessor::addRedirection(t, u);
    ASSERT_EQ(1,t.getRedirectionStack().size());
    ASSERT_TRUE(t.getRedirectionStack()[0] == u);
    t2 = t;
    t2= t2;
    ASSERT_EQ(1,t2.getRedirectionStack().size());
    ASSERT_TRUE(t2.getRedirectionStack()[0] == u);

    HttpCacheToken* t3,* t4;
    t3 = new HttpCacheToken();
    t4 = new HttpCacheToken(*t3);
    ASSERT_TRUE(t3 != NULL);
    ASSERT_TRUE(t4 != NULL);
    delete t3;
    delete t4;
}

