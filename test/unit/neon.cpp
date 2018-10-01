#include <davix.hpp>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <gtest/gtest.h>

using namespace Davix;

static const char* list_urls[] = {
    "http://localhost:80/",
    "sfdfsdfdsfds://localhost:rrrrrrr/path",
    "http://monurlrandom/",
    "www.google.com"
};

static const int list_port[]={
    80,
    0,
    0,
    80
};

static const char* list_host[] ={
    "localhost",
    "localhost",
    "monurlrandom",
    "nothing"
};

static const char* list_path[] ={
    "/",
    "/path",
    "/",
    "nothing",
};

static const char* list_proto[] ={
    "http",
    "http",
    "http",
    "nothing"
};

static bool failure[]={
    false,
    true,
    false,
    true
};

const size_t len_list= (sizeof(list_proto))/(sizeof(const char*));


TEST(testNeon, testParsing){
    davix_set_log_level(DAVIX_LOG_ALL);
    for(size_t i=0; i < len_list; ++i){
        Uri uri(list_urls[i]);
        if(failure[i] == false){
            ASSERT_EQ(StatusCode::OK, uri.getStatus());
            ASSERT_STREQ(list_proto[i], uri.getProtocol().c_str() );
            ASSERT_STREQ(list_host[i], uri.getHost().c_str());
            ASSERT_STREQ(list_path[i], uri.getPath().c_str());
            ASSERT_EQ(list_port[i], uri.getPort());
        }else{
            ASSERT_EQ(StatusCode::UriParsingError, uri.getStatus());
        }
    }
}


TEST(testUri, testCopyEQ){
    Uri uri;
    std::ostringstream ss;
    ASSERT_EQ(uri.getStatus(), StatusCode::UriParsingError);
    uri = Uri("http://wikipedia.org:80/bonus?salut");
    ASSERT_EQ(uri.getStatus(), StatusCode::OK);

    ss << uri;
    ASSERT_STREQ(ss.str().c_str(),uri.getString().c_str());

    std::string pathAndString("http://wikipedia.org:80/bonus?salut=bob");
    uri = pathAndString;
    std::string::reverse_iterator rit = std::find(pathAndString.rbegin(), pathAndString.rend(), '/');
    std::string::iterator it = pathAndString.begin() + (pathAndString.rend() - rit);
    ASSERT_STREQ(uri.getPathAndQuery().c_str(), std::string(it-1, pathAndString.end()).c_str() );

    Uri uri2(uri);
    ASSERT_TRUE(uri == uri2);
}



TEST(testUri, testEscape){

    std::string s("http://wikipedia.org:80/bonus a toi \\ é voilù ?salut");

    std::string r = Davix::Uri::unescapeString(Davix::Uri::escapeString(s));
    ASSERT_STREQ(s.c_str(), r.c_str());
}

TEST(testUri, testTrailingSlashes) {
    std::vector<std::string> vec;
    vec.push_back("https://wikipedia.org/wiki/Example");
    vec.push_back("http://example.org/test");

    std::vector<std::string>::iterator it;
    for(it = vec.begin(); it != vec.end(); it++) {
        std::string s = *it;
        std::string s2 = s + "/";

        Uri uri(s);
        ASSERT_EQ(uri.getString(), s);

        uri.ensureTrailingSlash();
        ASSERT_EQ(uri.getString(), s2);

        uri.ensureTrailingSlash();
        ASSERT_EQ(uri.getString(), s2);

        uri.removeTrailingSlash();
        ASSERT_EQ(uri.getString(), s);

        uri.removeTrailingSlash();
        ASSERT_EQ(uri.getString(), s);
    }
}

TEST(testUri, testQueryParamExists) {
    std::string s("https://example.com/somepath?query1=aaa&query2=bbb&query3=cccc&query4=ddd");
    Uri uri(s);
    ASSERT_TRUE(uri.queryParamExists("query1"));
    ASSERT_TRUE(uri.queryParamExists("query2"));
    ASSERT_TRUE(uri.queryParamExists("query3"));
    ASSERT_TRUE(uri.queryParamExists("query4"));

    ASSERT_FALSE(uri.queryParamExists("query5"));
    ASSERT_FALSE(uri.queryParamExists("query6"));
    ASSERT_FALSE(uri.queryParamExists("query"));
    ASSERT_FALSE(uri.queryParamExists("aaa"));
    ASSERT_FALSE(uri.queryParamExists("bbb"));
    ASSERT_FALSE(uri.queryParamExists("cccc"));
    ASSERT_FALSE(uri.queryParamExists("ccc"));
    ASSERT_FALSE(uri.queryParamExists("ddd"));
    ASSERT_FALSE(uri.queryParamExists("aaa&query2"));
}

TEST(testUri, testFragment) {
    Davix::Uri u("https://example.com/path?query1=aa&query2=bb#frag1=aa&frag2=bb");
    ASSERT_EQ(u.getFragment(), "frag1=aa&frag2=bb");
    ASSERT_TRUE(u.fragmentParamExists("frag1"));
    ASSERT_TRUE(u.fragmentParamExists("frag2"));
    ASSERT_FALSE(u.fragmentParamExists("frag3"));

    Davix::Uri u2("https://example.com/path?query1=aa&query2=bb");
    ASSERT_EQ(u2.getFragment(), "");
    ASSERT_FALSE(u2.fragmentParamExists("frag1"));

    Davix::Uri u3(u);
    ASSERT_EQ(u.getFragment(), u3.getFragment());

    Davix::Uri u4 = u;
    ASSERT_EQ(u.getFragment(), u4.getFragment());
}
