#include "test_httprequest.hpp"
#include <gtest/gtest.h>
#include <davix.hpp>

using namespace Davix;

TEST(testHttpCache, testCreate){
    davix_set_log_level(DAVIX_LOG_ALL);

    Uri u("http://higgs.boson/is/watchingus");
    Uri u_red("http://higgs.boson/is/watchingus/on/moon");



}

