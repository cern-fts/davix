#include <config.h>
#include "stringutils.hpp"
#include <cstring>




std::vector<std::string> stringTokSplit(const std::string & str, std::string delimiter){
    const size_t s_str = str.size();
    char * token, *input, *state;
    std::vector<std::string> res;
    char buffer[s_str +1];
    memcpy(buffer, str.c_str(), s_str);
    buffer[s_str] ='\0';

    for(input = buffer; ; input = NULL){
        token = strtok_r(input, delimiter.c_str(), &state);
        if(!token)
            break;
        res.push_back(token);
    }

    return res;
}


int string_compare_ncase(const std::string & str1, const std::string & str2){
    return strcasecmp(str1.c_str(), str2.c_str());
}


int string_compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2){
     return strncasecmp(str1.c_str(), cstr2+ offset, size);
}
