#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP



inline bool isValidHeaderChar(const char* p){
    return (*p != '\0'
            && *p != '\n'
            && *p != '\r'
            && *p != ' '
            && *p != '\t' );
}

#endif // HTTP_UTIL_HPP
