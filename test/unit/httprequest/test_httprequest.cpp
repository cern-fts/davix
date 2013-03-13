#include "test_httprequest.hpp"
#include <gtest/gtest.h>
#include <davix.hpp>
#include <request/httpcachetoken_internal.hpp>

using namespace Davix;

TEST(testHttpCacheToken, testCreate){
    davix_set_log_level(DAVIX_LOG_ALL);

    Uri u("http://higgs.boson/is/watchingus");
    Uri u_red("http://higgs.boson/is/watchingus/on/moon");

    // stupid creation
    HttpCacheToken t;
    HttpCacheToken t2(t);

    HttpCacheToken* td = HttpCacheTokenAccessor::createCacheToken(u, u_red);
    ASSERT_EQ(u, td->getrequestUri());
    ASSERT_EQ(u_red, td->getCachedRedirection());

    HttpCacheToken* td2 = new HttpCacheToken(*td);
    delete td;
    ASSERT_EQ(u, td2->getrequestUri());
    ASSERT_EQ(u_red, td2->getCachedRedirection());
    delete td2;
}

