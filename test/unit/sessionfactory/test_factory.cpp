#include <gtest/gtest.h>
#include <davix.hpp>
#include <neon/neonsessionfactory.hpp>

using namespace Davix;

TEST(testRedirectCache, testCacheSimple){
    davix_set_log_level(DAVIX_LOG_ALL);


    boost::shared_ptr<Uri> dest(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    boost::shared_ptr<Uri> dest2(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    Uri u("http://higgs.boson/is/watchingus");

    Uri u_sec("https://higgs.boson/is/watchingus");
    Uri u_port("http://higgs.boson:8668/is/watchingus");

    NEONSessionFactory f;
    f.addRedirection("GET", u, dest);
    ASSERT_TRUE(f.redirectionResolve("GET", u) == dest);
    ASSERT_TRUE(f.redirectionResolve("GET", u_sec).get() == NULL);
    ASSERT_TRUE(f.redirectionResolve("GET", u_port).get() == NULL);

    // add redirection
    f.addRedirection("GET", u_port, dest2);
    ASSERT_TRUE(f.redirectionResolve("GET", u) != dest2);
    ASSERT_TRUE(f.redirectionResolve("GET", u_port) == dest2);

    // remove redirection
    f.redirectionClean("GET", u);
    ASSERT_TRUE(f.redirectionResolve("GET", u).get() == NULL);
    ASSERT_TRUE(f.redirectionResolve("GET", u_port) == dest2);
}

