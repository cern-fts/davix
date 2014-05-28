#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <cstring>


typedef std::vector<std::string> stringVec;

namespace StrUtil{


inline bool isslash(int c){
    return ( c == '/');
}

inline bool isCrLf(int c){
    return (( c == '\n') || (c == '\r'));
}

inline bool isSpace(int c){
    return static_cast<bool>(::isspace(static_cast<char>(c)));
}

// predicates
inline bool charEqCase(char c1, char c2){
    return (c1 == c2 || ::tolower(c1) == ::tolower(c2));
}



/////////////////////////////
// Parsing
/////////////////////////////
//split a string following an array of delimiter ( strtok like )
std::vector<std::string> tokenSplit(const std::string & str, const std::string & delimiter);

inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &res) {
    std::stringstream ss(s);
    std::string item;
    item.reserve(s.size());
    while (std::getline(ss, item, delim)) {
        res.push_back(item);
    }
    return res;
}


inline std::string & remove(std::string & str, char c){
    std::string::iterator it = std::remove(str.begin(), str.end(), c);
    str.erase(it, str.end());
    return str;
}



///////////////////////////
/// compare utils
//////////////////////
// compare two C++ string case insensitive
int compare_ncase(const std::string & str1, const std::string & str2);

int compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2);



inline size_t copy_std_string_to_buff(char* buffer, size_t max_size, const std::string & str){
    const size_t str_size = str.copy(buffer, max_size-1);
    buffer[str_size]= '\0';
    return str_size;
}

///////////////////////////////////
///// String utilities
//////////////////////////////////

// trim from start
template <typename Func>
inline std::string &ltrim(std::string &s, Func  pred) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, bool>(pred))));
        return s;
}

inline std::string &ltrim(std::string &s) {
        return ltrim(s, isSpace);
}

// trim from end
template <typename Func>
inline std::string &rtrim(std::string &s, Func pred) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, bool>(pred))).base(), s.end());
        return s;
}

inline std::string &rtrim(std::string &s) {
    return rtrim(s, isSpace);
}

// trim from both
template <typename Func>
inline std::string &trim(std::string &s, Func pred) {
    return ltrim<Func>(rtrim<Func>(s, pred), pred);
}

inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}


}


#endif // STRINGUTILS_HPP
