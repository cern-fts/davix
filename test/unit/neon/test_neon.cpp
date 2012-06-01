

#include <core.h>
#include <abstractsessionfactory.h>
#include <neon/neonsessionfactory.h>
#include <string>
#include <cstring>
#include "test_neon.h"

using namespace Davix;

static const char* list_urls[] = {
    "http://localhost:80/",
    "http://localhost:rrrrrrr/path" /* error*/
};

static const int list_port[]={
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

void test_url_parsing(){
    g_logger_set_globalfilter(G_LOG_LEVEL_MASK);
    std::string url, host, path, proto;
    unsigned long port;
    for(int i=0; i < len_list; ++i){
        if(failure[i] == false){
            parse_http_neon_url(std::string(list_urls[i]), proto, host, path, &port);
            assert_true_with_message( proto.compare(std::string(list_proto[i]))==0, " proto FAIL");
            assert_true_with_message( host.compare(list_host[i])==0, " list FAIL");
            assert_true_with_message( path.compare(list_path[i])==0, " path FAIL");
            assert_true_with_message( port == list_port[i], " port FAIL %d %d", port, list_port[i]);
        }else{
            try{
              parse_http_neon_url(std::string(list_urls[i]), proto, host, path, &port);
              assert_true_with_message(FALSE, "must not reach this, exception ! ");
            }catch(Glib::Error & e){

            }
        }
    }
}


TestSuite * neon_suite (void)
{
        TestSuite *s2 = create_test_suite();
        // verbose test case /
        add_test(s2, test_url_parsing);

        return s2;
 }
