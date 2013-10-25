#include <davix.hpp>
#include <string>
#include <cstring>
#include <gtest/gtest.h>

using namespace Davix;

static const char* list_urls[] = {
    "http://localhost:80/",
    "sfdfsdfdsfds://localhost:rrrrrrr/path",
    "http://monurlrandom/",
    "www.google.com"
};

static const size_t list_port[]={
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

const size_t len_list=4;


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


