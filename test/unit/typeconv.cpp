
#include <iostream>
#include <string>
#include <cstring>
#include <gtest/gtest.h>

#undef A_LIB_NAMESPACE
#define A_LIB_NAMESPACE Davix


#include "libs/alibxx/alibxx.hpp"



using namespace std;

struct DummyStruct{
    std::string dude;
};


// instanciate and play with gates
TEST(ALibxx, StringToULong){
    using namespace Davix;
    std::string str_ulong("123456");
    toType<unsigned long, std::string> conv;

    unsigned long ul = conv(str_ulong);

    ASSERT_EQ(ul, 123456);

    str_ulong = "0";
    ul= conv(str_ulong);
    ASSERT_EQ(ul, 0);

   try{
        std::string str2 = "random string";
        Davix::toType<unsigned long, std::string>()(str2);
        FAIL();
    }catch(TypeConvException & e){
        // silent
    }


    try{
         std::string str2 = "-15";
         Davix::toType<unsigned long, std::string>()(str2);
         FAIL();
     }catch(TypeConvException & e){
         // silent
     }

    unsigned long long super_long = std::numeric_limits<unsigned long long>::max();
    std::ostringstream ss;
    ss << super_long;
    ss << super_long; // overflow
    try{
            ul =toType<unsigned long, std::string>()(ss.str());
            FAIL();
    }catch(...){
        // silent
    }

}


// instanciate and play with gates
TEST(ALibxx, StringToLong){
    using namespace Davix;
    std::string str_long("123456");
    toType<long, std::string> conv;

    long l = conv(str_long);

    ASSERT_EQ(l, 123456);

    str_long = "0";
    l= conv(str_long);
    ASSERT_EQ(l, 0);

   try{
        std::string str2 = "random string";
        Davix::toType<long, std::string>()(str2);
        FAIL();
    }catch(TypeConvException & e){
        // silent
    }


    str_long = "-9865743";
    l = conv(str_long);
    ASSERT_EQ(l, -9865743);

}



// instanciate and play with gates
TEST(ALibxx, StringToInt){
    using namespace Davix;
    std::string str_int("123456");
    toType<int, std::string> conv;

    int l = conv(str_int);

    ASSERT_EQ(l, 123456);

    str_int = "0";
    l= conv(str_int);
    ASSERT_EQ(l, 0);

   try{
        std::string str2 = "random string";
        toType<int, std::string>()(str2);
        FAIL();
    }catch(TypeConvException & e){
        // silent
    }


    str_int = "-9865743";
    l = conv(str_int);
    ASSERT_EQ(l, -9865743);


    long long super_long = std::numeric_limits<long long>::max();
    std::ostringstream ss;
    ss << super_long;
    ss << super_long; // overflow
    try{
            l =toType<int, std::string>()(ss.str());
            FAIL();
    }catch(...){
        // silent
    }
}
