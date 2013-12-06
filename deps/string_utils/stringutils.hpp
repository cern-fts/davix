#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <cstring>

// split a string following an array of delimiter ( strtok like )
std::vector<std::string> stringTokSplit(const std::string & str, const std::string & delimiter);

// compare two C++ string case insensitive
int string_compare_ncase(const std::string & str1, const std::string & str2);

int string_compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2);


inline size_t copy_std_string_to_buff(char* buffer, size_t max_size, const std::string & str){
    const size_t str_size = str.copy(buffer, max_size-1);
    buffer[str_size]= '\0';
    return str_size;
}




// trim from start
template <typename Func>
inline std::string &ltrim(std::string &s, Func  pred = static_cast<int (*)(int)>(std::isspace)) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(pred))));
        return s;
}


// trim from end
template <typename Func>
inline std::string &rtrim(std::string &s, Func pred = std::isspace) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(pred))).base(), s.end());
        return s;
}

// trim from both
template <typename Func>
inline std::string &trim(std::string &s, Func pred = std::isspace) {
        return ltrim<Func>(rtrim<Func>(s));
}

inline int isslash(int c){
    return ( c == '/');
}


#endif // STRINGUTILS_HPP
