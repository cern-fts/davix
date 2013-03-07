#include <config.h>
#include "stringutils.hpp"
#include <cstring>


#if !(defined HAVE_STRTOK_R) && !(defined HAVE_STRTOK_S)

static char* __internal_strtok_r(
    char *str,
    const char *delim,
    char **nextp)
{
    char *ret;

    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}

#endif


std::vector<std::string> stringTokSplit(const std::string & str, const std::string & delimiter){
    const size_t s_str = str.size();
    char * token, *input, *state;
    std::vector<std::string> res;
    char buffer[s_str +1];
    memcpy(buffer, str.c_str(), s_str);
    buffer[s_str] ='\0';


    for(input = buffer; ; input = NULL){
#if defined HAVE_STRTOK_R
        token = strtok_r(input, delimiter.c_str(), &state);
#elif defined HAVE_STRTOK_S
        token = strtok_s(input, delimiter.c_str(), &state);

#else
        token = __internal_strtok_r(input, delimiter.c_str(), &state);
#endif
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
