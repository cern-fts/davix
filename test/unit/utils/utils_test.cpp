#include "utils_test.hpp"

#include <davix.hpp>
#include <string_utils/stringutils.hpp>
#include <base64/base64.hpp>
#include <hmac_sha1/hmacsha1.hpp>
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

    str = " Obi;wan Kenobi.droid*pass$*";
    delimiter=" ;.*$ù^°=";
    res = stringTokSplit(str, delimiter);
    ASSERT_EQ(5, res.size());
    ASSERT_STREQ("Obi", res.at(0).c_str());
    ASSERT_STREQ("Kenobi", res.at(2).c_str());
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


TEST(testhmacsha1, testhmac){

    const std::string data("obi wan kenobi");
    const std::string key("bob dylan");
    const std::string result("337a4432486ea5a175c35ed1a138d6f9dd481f15");

    const std::string prod = hmac_sha1(key, data);
    std::ostringstream ss;
    ss << std::hex << prod;

    ASSERT_STREQ(prod.c_str(), ss.str().c_str());

}


TEST(testS3, test_hash_s3){

    const std::string key= "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY";
    const std::string str("GET\n"
                          "\n"
                          "\n"
                          "Tue, 27 Mar 2007 19:36:42 +0000\n"
                          "/johnsmith/photos/puppy.jpg");

    const std::string res("bWq2s1WEIj+Ydj0vQ697zp+IXMU=");


    const std::string hmac_str = hmac_sha1(key,str);
    const std::string prod = Base64::base64_encode((unsigned char*) hmac_str.c_str(), hmac_str.size());
    std::cout << "hash : " << prod << std::endl;


    ASSERT_STREQ(res.c_str(), prod.c_str());

}


