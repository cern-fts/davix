#include <gtest/gtest.h>
#include <davix.hpp>
#include <neon/neonsessionfactory.hpp>
#include <core/RedirectionResolver.hpp>

using namespace Davix;

TEST(testRedirectCache, testCacheSimple){
    davix_set_log_level(DAVIX_LOG_ALL);


    std::shared_ptr<Uri> dest(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    std::shared_ptr<Uri> dest2(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    Uri u("http://higgs.boson/is/watchingus");

    Uri u_sec("https://higgs.boson/is/watchingus");
    Uri u_port("http://higgs.boson:8668/is/watchingus");

    RedirectionResolver f(true);
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



TEST(testRedirectCache, testCacheChainRedirection){
    davix_set_log_level(DAVIX_LOG_ALL);

    Uri u("http://higgs.boson/is/watchingus");

    std::shared_ptr<Uri> url1(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    std::shared_ptr<Uri> url2(new Uri("http://server2.com/dsffds/sfdfdsfsdfdsfdsfds"));
    std::shared_ptr<Uri> url3(new Uri("http://server2.com:8080/dsffds/sfdfdsfsdfdsfdsfds"));
    std::shared_ptr<Uri> url4(new Uri("http://server3.com/dsffds/fsdaaaaa"));

    RedirectionResolver f(true);
    f.addRedirection("GET", u, url1);
    f.addRedirection("GET",*url1, url2);
    f.addRedirection("GET", *url2, url3);
    f.addRedirection("GET", *url3, url4);

    ASSERT_TRUE(f.redirectionResolve("GET", u) == url4);

    ASSERT_TRUE(f.redirectionResolve("GET", *url2) == url4);
    f.redirectionClean("GET", u);
    ASSERT_TRUE(f.redirectionResolve("GET", u).get() == NULL);
    ASSERT_TRUE(f.redirectionResolve("GET", *url2).get() == NULL);
    ASSERT_TRUE(f.redirectionResolve("GET", *url4).get() == NULL);
}


TEST(testRedirectCache, test_GET_HEAD){
    davix_set_log_level(DAVIX_LOG_ALL);

    Uri u("http://higgs.boson/is/watchingus");

    std::shared_ptr<Uri> url1(new Uri("http://sffsdfsd.com/dsffds/fsdfsdsdf"));
    std::shared_ptr<Uri> url2(new Uri("http://server2.com/dsffds/sfdfdsfsdfdsfdsfds"));

    RedirectionResolver f(true);
    f.addRedirection("GET", u, url1);

    ASSERT_TRUE(f.redirectionResolve("GET", u) == url1);
    ASSERT_TRUE(f.redirectionResolve("HEAD", u) == url1);
    ASSERT_TRUE(f.redirectionResolve("PUT", u).get() == NULL);
    f.redirectionClean("GET", u);

    ASSERT_TRUE(f.redirectionResolve("GET", u) == NULL);
    ASSERT_TRUE(f.redirectionResolve("HEAD", u) == NULL);
    f.addRedirection("PUT", u, url1);
    ASSERT_TRUE(f.redirectionResolve("GET", u).get() == NULL);
    ASSERT_TRUE(f.redirectionResolve("PUT", u) == url1);

    f.addRedirection("HEAD", u, url1);

    ASSERT_TRUE(f.redirectionResolve("GET", u) == url1);
    ASSERT_TRUE(f.redirectionResolve("HEAD", u) == url1);
}
