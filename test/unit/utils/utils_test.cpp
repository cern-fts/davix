#include "utils_test.hpp"

#include <davix.hpp>
#include <string_utils/stringutils.hpp>
#include <base64/base64.hpp>
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


TEST(testBase64, cmpbase){

    size_t s_buff= rand()%100000;
    char buff_input[s_buff];

    for(size_t i = 0; i < s_buff; i++)
        buff_input[i]= (char) rand()%255;

    std::string conv = Base64::base64_encode((unsigned char*)buff_input, s_buff);

  //  std::cout << conv << std::endl;

    std::string res=  Base64::base64_decode(conv);
    ASSERT_EQ(s_buff, res.length());
    ASSERT_TRUE( memcmp(buff_input, res.c_str(), s_buff) == 0);

}
