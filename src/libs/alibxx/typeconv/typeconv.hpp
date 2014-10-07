#pragma once

#include <string>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <cerrno>

#include "typeconv_exception.hpp"
#include "../base_types.hpp"


namespace A_LIB_NAMESPACE{

namespace{
template<typename Unsigned, typename Signed>
void check_sign(Unsigned val, const std::string & orig_string){
    if( val > static_cast<Unsigned>(std::numeric_limits<Signed>::max()) && orig_string.find('-') != std::string::npos){
        throw TypeConvException("Negative value to unsigned value");
    }
}

template<typename SourceType, typename DestType>
DestType numerical_cast_safe(SourceType val){
    if(val < static_cast<SourceType>(std::numeric_limits<DestType>::min())
            || val > static_cast<SourceType>(std::numeric_limits<DestType>::max())){
        throw TypeConvException("Overflow in numerical conversion");
    }
    return static_cast<DestType>(val);
}

}


template<typename T, typename S>
struct toType{
    T operator()(const S & val){
            (void) val;
            throw TypeConvException("Invalid type converstion: no specialization");
    }

};


template <>
struct toType<unsigned long long, std::string>{
    unsigned long long operator()(const std::string & str){
        char* end_str = NULL;
        const unsigned long long ret = strtoull(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to unsigned long long");
        }
        check_sign<unsigned long long, long long>(ret, str);
        return ret;
    }
};


template <>
struct toType<long long, std::string>{
    long long operator()(const std::string & str){
        char* end_str = NULL;
        const long long ret = strtoll(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to long long");
        }
        return ret;
    }
};





template <>
struct toType<unsigned long, std::string>{
    unsigned long operator()(const std::string & str){
        char* end_str = NULL;
        const unsigned long ret = strtoul(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to unsigned long");
        }
        check_sign<unsigned long, long>(ret, str);
        return ret;
    }
};


template <>
struct toType<long, std::string>{
    unsigned long operator()(const std::string & str){
        char* end_str = NULL;
        const long ret = strtol(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to long");
        }
        return ret;
    }
};


template <>
struct toType<unsigned int, std::string>{
    unsigned int operator()(const std::string & str){
        char* end_str = NULL;
        const unsigned long ret = strtoul(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to unsigned");
        }
        check_sign<unsigned long, long>(ret, str);
        return numerical_cast_safe<unsigned long, unsigned int>(ret);
    }
};



template <>
struct toType<int, std::string>{
    int operator()(const std::string & str){
        char* end_str = NULL;
        const long ret = strtol(str.c_str(), &end_str, 10);
        errno =0;
        if( str.size() ==0 || *end_str != '\0'){
             throw TypeConvException("Invalid type converstion from string to int");
        }
        return numerical_cast_safe<long, int>(ret);
    }
};


/*
template<typename T>
struct toType<T, std::string>{
    T operator()(const std::string  & val){
        T ret;
        ss.clear();
        ss.str(val);
        ss >> ret;
        if(ss.fail()){
            throw TypeConvException("Invalid type converstion from string to generic");
        }
        return ret;
    }

    std::istringstream ss;
};
*/

template<typename S>
struct toType<std::string, S>{
    std::string operator()(const S & val){
        S ret;
        ss.str("");
        ss << val;
        if(ss.fail()){
            throw TypeConvException("Invalid type converstion from string to generic");
        }
        return ret.str();
    }

    std::ostringstream ss;
};

}
