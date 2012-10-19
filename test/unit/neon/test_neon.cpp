

#include <davixcontext.hpp>
#include <abstractsessionfactory.hpp>
#include <neon/neonsessionfactory.hpp>
#include <string>
#include <cstring>
#include <gtest/gtest.h>

using namespace Davix;

static const char* list_urls[] = {
    "http://localhost:80/",
    "http://localhost:rrrrrrr/path" /* error*/
};

static const size_t list_port[]={
    80,
    8080
};

static const char* list_host[] ={
    "localhost",
    "localhost"
};

static const char* list_path[] ={
    "/",
    "/path"
};

static const char* list_proto[] ={
    "http",
    "http"
};

static bool failure[]={
    false,
    true
};

const size_t len_list=2;


TEST(testNeon, testParsing){
    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);
    std::string url, host, path, proto;
    unsigned long port;
    for(size_t i=0; i < len_list; ++i){
        if(failure[i] == false){
            parse_http_neon_url(std::string(list_urls[i]), proto, host, path, &port);
            ASSERT_EQ(0, proto.compare(std::string(list_proto[i])));
            ASSERT_EQ(0, host.compare(list_host[i]));
            ASSERT_EQ(0,path.compare(list_path[i]));
            ASSERT_EQ(list_port[i], port);
        }else{
            ASSERT_THROW({
              parse_http_neon_url(std::string(list_urls[i]), proto, host, path, &port);
            }, Glib::Error);
        }
    }
}


