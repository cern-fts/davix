#include "utils_test.hpp"

#include <davix.hpp>
#include <string_utils/stringutils.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Davix;

TEST(StringUtils, splitok){
    std::string str, delimiter;
    str= "hello world test";
    delimiter= " ";
    std::vector<std::string> res = stringTokSplit(str, delimiter);
    ASSERT_EQ(3, res.size());
    ASSERT_STREQ("hello", res.at(0).c_str());

    str=" bytes 0-90/15872   \t";
    delimiter="bytes -/\t";
    res = stringTokSplit(str, delimiter);
    ASSERT_EQ(3, res.size());
    ASSERT_STREQ("0", res.at(0).c_str());
    ASSERT_STREQ("90", res.at(1).c_str());
    ASSERT_STREQ("15872", res.at(2).c_str());


    str="      ";
    delimiter=" ";
    res = stringTokSplit(str, delimiter);
    ASSERT_EQ(0, res.size());
}
