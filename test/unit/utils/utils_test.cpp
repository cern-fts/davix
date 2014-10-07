#include "utils_test.hpp"

#include <davix.hpp>
#include <string_utils/stringutils.hpp>
#include <tools/davix_tool_util.hpp>
#include <base64/base64.hpp>
#include <hmac_sha1/hmacsha1.hpp>
#include <utils/davix_s3_utils.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Davix;
using namespace StrUtil;

TEST(StringUtils, splitok){
    std::string str, delimiter;
    str= "hello world test";
    delimiter= " ";
    std::vector<std::string> res = tokenSplit(str, delimiter);
    ASSERT_EQ(3, res.size());
    ASSERT_STREQ("hello", res.at(0).c_str());

    str=" bytes 0-90/15872   \t";
    delimiter="bytes -/\t";
    res = tokenSplit(str, delimiter);
    ASSERT_EQ(3, res.size());
    ASSERT_STREQ("0", res.at(0).c_str());
    ASSERT_STREQ("90", res.at(1).c_str());
    ASSERT_STREQ("15872", res.at(2).c_str());

    str = " Obi;wan Kenobi.droid*pass$*";
    delimiter=" ;.*$ù^°=";
    res = tokenSplit(str, delimiter);
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

TEST(testStringMode, test_mode){
    mode_t m = 0755;
    string m_str = Tool::string_from_mode(m);
    //std::cout << m_str << std::endl;
    ASSERT_STREQ("-rwxr-xr-x", m_str.c_str());

    m =  040777;
    m_str = Tool::string_from_mode(m);
    ASSERT_STREQ("drwxrwxrwx", m_str.c_str());
}


TEST(testAuthS3, ReqToSign){
    RequestParams params;
    Uri url("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;
    vec.push_back(std::pair<std::string,std::string>("Date", "Tue, 27 Mar 2007 19:36:42 +0000"));

    S3::signRequest(params, "GET", url, vec);
    ASSERT_EQ(std::string("Authorization"),vec.at(1).first);
    ASSERT_EQ(std::string("AWS AKIAIOSFODNN7EXAMPLE:bWq2s1WEIj+Ydj0vQ697zp+IXMU="),vec.at(1).second);
}


TEST(testAuthS3, ReqToToken){
    RequestParams params;
    Uri url("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;

    Uri u = S3::tokenizeRequest(params, "GET", url, vec, static_cast<time_t>(1175139620UL));
    Uri resu("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg?AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE&Signature=NpgCjnDzrM%2BWFzoENXmpNDUsSn8%3D&Expires=1175139620");
    ASSERT_TRUE(StrUtil::compare_ncase(resu.getString(), u.getString()) ==0);
}


