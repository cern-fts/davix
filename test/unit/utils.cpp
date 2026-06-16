#include <davix.hpp>
#include <utils/stringutils.hpp>
#include <tools/davix_tool_util.hpp>
#include "libs/alibxx/crypto/base64.hpp"
#include "libs/alibxx/crypto/hmacsha.hpp"
#include <utils/davix_s3_utils.hpp>
#include <utils/davix_swift_utils.hpp>
#include <gtest/gtest.h>
#include <core/SessionPool.hpp>
#include <curl/HeaderlineParser.hpp>

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

TEST(testAuthS3, ReqToSignPostDelete){
    RequestParams params;
    Uri url("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg?delete");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;
    vec.push_back(std::pair<std::string,std::string>("Date", "Tue, 27 Mar 2007 19:36:42 +0000"));

    S3::signRequest(params, "POST", url, vec);
    ASSERT_EQ(std::string("Authorization"),vec.at(1).first);
    ASSERT_EQ("AWS AKIAIOSFODNN7EXAMPLE:F/FLAv2x9llsMYvDJ79Sw2MuByU=", vec.at(1).second);
}

TEST(testAuthS3, ReqToSignAWS){
    RequestParams params;
    Uri url("http://static.johnsmith.net:8080/db-backup.dat.gz");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;
    vec.push_back(HeaderLine("Date", "Tue, 27 Mar 2007 21:06:08 +0000"));
    vec.push_back(HeaderLine("x-amz-acl","public-read"));
    vec.push_back(HeaderLine("X-Amz-Meta-ReviewedBy", "joe@johnsmith.net,jane@johnsmith.net"));
    vec.push_back(HeaderLine("X-Amz-Meta-FileChecksum","0x02661779"));
    vec.push_back(HeaderLine("X-Amz-Meta-ChecksumAlgorithm", "crc32"));
    vec.push_back(HeaderLine("Content-Disposition","attachment; filename=database.dat"));
    vec.push_back(HeaderLine("Content-Encoding","gzip"));
    vec.push_back(HeaderLine("Content-Length","5913339"));

    S3::signRequest(params, "PUT", url, vec);
    ASSERT_EQ(std::string("Authorization"),vec.back().first);
    ASSERT_EQ(std::string("AWS AKIAIOSFODNN7EXAMPLE:mRp45AGRkcT9u0ssDHIkjUqmPWk="),vec.back().second);
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

TEST(testAuthS3, ReqToTokenDelete){
    RequestParams params;
    Uri url("http://johnsmith.s3.amazonaws.com/photos/puppy.jpg?delete");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;

    Uri u = S3::tokenizeRequest(params, "POST", url, vec, static_cast<time_t>(1175139620UL));
    ASSERT_EQ(u.getString(), "http://johnsmith.s3.amazonaws.com/photos/puppy.jpg?delete&AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE&Signature=fU7FkNg8QPiGj8YE62xbZuac6dQ%3D&Expires=1175139620");
}

TEST(testAuthS3, ReqToTokenWithHeaders){
    RequestParams params;
    Uri url("http://firwen-bucket.s3.amazonaws.com/testfile1234");
    params.setAwsAuthorizationKeys("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");


    HeaderVec vec;
    vec.push_back(HeaderLine("x-amz-meta-fed-acl", "adevress : rwx, furano : rwx"));

    Uri u = S3::tokenizeRequest(params, "PUT", url, vec, static_cast<time_t>(1415835686));
    Uri resu("http://firwen-bucket.s3.amazonaws.com/testfile1234?AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE&Signature=8DnY%2F3Te1GOcC01S6BGNHZErJMo%3d&Expires=1415835686&x-amz-meta-fed-acl=adevress%20%3a%20rwx%2c%20furano%20%3a%20rwx");
    std::cout << u << "\n" << resu << std::endl;
    ASSERT_TRUE(StrUtil::compare_ncase(resu.getString(), u.getString()) ==0);
}

TEST(testAuthSwift, signUri){
    RequestParams params;
    Uri url("https://hostname.com/containersth2873/objectfile1234");
    params.setOSProjectID("21e698ff1238438fabc72e5cf9d59165");

    Uri u = Swift::signURI(params, url);
    ASSERT_EQ(u.getString(), "https://hostname.com/v1/AUTH_21e698ff1238438fabc72e5cf9d59165/containersth2873/objectfile1234");
}

TEST(CanonicalizedResourceQueryParams, BasicSanity) {
    //Uri url(

}

TEST(SessionPool, BasicSanity) {
    SessionPool<int> pool;

    int out;

    pool.insert("test-1", 3);
    ASSERT_FALSE(pool.retrieve("test", out));

    ASSERT_TRUE(pool.retrieve("test-1", out));
    ASSERT_EQ(out, 3);

    ASSERT_FALSE(pool.retrieve("test-1", out));


    pool.insert("test-2", 3);
    pool.insert("test-2", 4);
    pool.insert("test-2", 3);
    pool.insert("test-2", 5);

    ASSERT_TRUE(pool.retrieve("test-2", out));
    ASSERT_EQ(out, 3);

    ASSERT_TRUE(pool.retrieve("test-2", out));
    ASSERT_EQ(out, 4);

    ASSERT_TRUE(pool.retrieve("test-2", out));
    ASSERT_EQ(out, 3);

    ASSERT_TRUE(pool.retrieve("test-2", out));
    ASSERT_EQ(out, 5);
}

TEST(HeaderlineParser, BasicSanity) {
    HeaderlineParser parser("");
    ASSERT_EQ(parser.getKey(), "");
    ASSERT_EQ(parser.getValue(), "");

    std::string buff("12345");
    parser = HeaderlineParser(buff.c_str(), buff.size()+1);
    ASSERT_EQ(parser.getKey(), "12345");
    ASSERT_EQ(parser.getValue(), "");

    parser = HeaderlineParser("test");
    ASSERT_EQ(parser.getKey(), "test");
    ASSERT_EQ(parser.getValue(), "");

    parser = HeaderlineParser("aaa: bbb");
    ASSERT_EQ(parser.getKey(), "aaa");
    ASSERT_EQ(parser.getValue(), "bbb");

    parser = HeaderlineParser("aaa:bbb");
    ASSERT_EQ(parser.getKey(), "aaa");
    ASSERT_EQ(parser.getValue(), "bbb");

    parser = HeaderlineParser("aaa:                             bbb");
    ASSERT_EQ(parser.getKey(), "aaa");
    ASSERT_EQ(parser.getValue(), "bbb");

    parser = HeaderlineParser("aaa: bbb\r\n");
    ASSERT_EQ(parser.getKey(), "aaa");
    ASSERT_EQ(parser.getValue(), "bbb");

    parser = HeaderlineParser("aaa:                             bbb\r\n");
    ASSERT_EQ(parser.getKey(), "aaa");
    ASSERT_EQ(parser.getValue(), "bbb");
}

// GOLDEN VECTOR — exact-match against AWS's published "GET Object" example.
//
// Source: AWS S3 Developer Guide, "Signature Calculations for the Authorization
// Header: Transferring Payload in a Single Chunk (AWS Signature Version 4)",
// Example: GET Object. Inputs and expected outputs are copied verbatim from
// that page. The expected signature is AWS's, not ours: if this test fails,
// fix the signer, never edit the expected value.
//
//   GET https://examplebucket.s3.amazonaws.com/test.txt
//   Range: bytes=0-9
//   x-amz-date: 20130524T000000Z   (pinned via the date seam)
//   region us-east-1, empty payload (x-amz-content-sha256 = SHA256(""))
//   SignedHeaders = host;range;x-amz-content-sha256;x-amz-date
TEST(testAuthS3, ReqToSignV4GoldenVector){
    RequestParams params;
    Uri url("https://examplebucket.s3.amazonaws.com/test.txt");
    ASSERT_EQ(url.getPath(), "/test.txt") << "path is: [" << url.getPath() << "]";
    params.setAwsAuthorizationKeys(
        "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY",
        "AKIAIOSFODNN7EXAMPLE");
    params.setAwsRegion("us-east-1");

    HeaderVec vec;
    // Pin the date (test seam) and supply Range exactly as the AWS example does.
    vec.push_back(HeaderLine("x-amz-date", "20130524T000000Z"));
    vec.push_back(HeaderLine("Range", "bytes=0-9"));

    S3::signRequest(params, "GET", url, vec);

    std::string auth;
    for(const auto& h : vec) {
        if(StrUtil::compare_ncase(h.first, "Authorization") == 0) auth = h.second;
    }
    ASSERT_FALSE(auth.empty()) << "no Authorization header produced";

    // Structural: exact SignedHeaders set & order, and credential scope.
    ASSERT_NE(auth.find("SignedHeaders=host;range;x-amz-content-sha256;x-amz-date"),
              std::string::npos) << "Authorization was: " << auth;
    ASSERT_NE(auth.find("Credential=AKIAIOSFODNN7EXAMPLE/20130524/us-east-1/s3/aws4_request"),
              std::string::npos) << "Authorization was: " << auth;

    // EXACT signature from AWS's published GET Object example.
    const std::string AWS_EXPECTED_SIGNATURE =
        "Signature=f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41";
    ASSERT_NE(auth.find(AWS_EXPECTED_SIGNATURE), std::string::npos)
        << "Signature mismatch — signer is wrong, do not edit the expected value.\n"
        << "Authorization was: " << auth;

    // Full Authorization header AWS publishes for this example (belt-and-suspenders).
    const std::string AWS_EXPECTED_AUTH =
        "AWS4-HMAC-SHA256 "
        "Credential=AKIAIOSFODNN7EXAMPLE/20130524/us-east-1/s3/aws4_request, "
        "SignedHeaders=host;range;x-amz-content-sha256;x-amz-date, "
        "Signature=f0e8bdb87c964420e857bd35b5d6ed310bd44f0170aba48dd91039c6036bdb41";
    ASSERT_EQ(auth, AWS_EXPECTED_AUTH)
        << "Full Authorization header does not match AWS's published value.";
}

// Determinism: same pinned inputs -> identical signature on repeated calls.
// (Catches accidental nondeterminism in canonicalization.)
TEST(testAuthS3, ReqToSignV4Deterministic){
    auto sign_once = [](){
        RequestParams params;
        Uri url("https://examplebucket.s3.amazonaws.com/test.txt");
        params.setAwsAuthorizationKeys(
            "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");
        params.setAwsRegion("us-east-1");
        HeaderVec vec;
        vec.push_back(HeaderLine("x-amz-date", "20130524T000000Z"));
        S3::signRequest(params, "GET", url, vec);
        for(const auto& h : vec)
            if(StrUtil::compare_ncase(h.first, "Authorization") == 0) return h.second;
        return std::string();
    };
    ASSERT_EQ(sign_once(), sign_once());
}

// A caller-supplied x-amz-date / x-amz-content-sha256
// must not appear twice in SignedHeaders.
TEST(testAuthS3, ReqToSignV4NoDuplicateSignedHeaders){
    RequestParams params;
    Uri url("https://examplebucket.s3.amazonaws.com/test.txt");
    params.setAwsAuthorizationKeys(
        "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");
    params.setAwsRegion("us-east-1");

    HeaderVec vec;
    vec.push_back(HeaderLine("x-amz-date", "20130524T000000Z"));
    vec.push_back(HeaderLine("x-amz-content-sha256",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

    S3::signRequest(params, "GET", url, vec);

    std::string auth;
    for(const auto& h : vec)
        if(StrUtil::compare_ncase(h.first, "Authorization") == 0) auth = h.second;

    // "x-amz-date" / "x-amz-content-sha256" must appear exactly once in SignedHeaders.
    const std::string sh_key = "SignedHeaders=";
    auto sh_pos = auth.find(sh_key);
    ASSERT_NE(sh_pos, std::string::npos);
    auto sh_end = auth.find(',', sh_pos);
    const std::string signed_list = auth.substr(sh_pos, sh_end - sh_pos);

    auto count_occurrences = [&](const std::string& needle){
        size_t n = 0, p = 0;
        while((p = signed_list.find(needle, p)) != std::string::npos){ ++n; p += needle.size(); }
        return n;
    };
    ASSERT_EQ(count_occurrences("x-amz-date"), 1u);
    ASSERT_EQ(count_occurrences("x-amz-content-sha256"), 1u);
}

// Verb behaviour: bodyless verbs sign the empty-SHA256; PUT signs
// UNSIGNED-PAYLOAD. Assert the x-amz-content-sha256 header reflects this.
TEST(testAuthS3, ReqToSignV4PayloadHashByVerb){
    auto content_sha = [](const std::string& verb){
        RequestParams params;
        Uri url("https://examplebucket.s3.amazonaws.com/test.txt");
        params.setAwsAuthorizationKeys(
            "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY", "AKIAIOSFODNN7EXAMPLE");
        params.setAwsRegion("us-east-1");
        HeaderVec vec;
        vec.push_back(HeaderLine("x-amz-date", "20130524T000000Z"));
        S3::signRequest(params, verb, url, vec);
        for(const auto& h : vec)
            if(StrUtil::compare_ncase(h.first, "x-amz-content-sha256") == 0) return h.second;
        return std::string();
    };
    const std::string EMPTY_SHA256 =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    ASSERT_EQ(content_sha("GET"), EMPTY_SHA256);
    ASSERT_EQ(content_sha("HEAD"), EMPTY_SHA256);
    ASSERT_EQ(content_sha("DELETE"), EMPTY_SHA256);
    ASSERT_EQ(content_sha("PUT"), std::string("UNSIGNED-PAYLOAD"));
}