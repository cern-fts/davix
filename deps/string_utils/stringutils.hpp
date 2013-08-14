#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <vector>
#include <string>
#include <cstring>

// split a string following an array of delimiter ( similar to strtok )
std::vector<std::string> stringTokSplit(const std::string & str, const std::string & delimiter);

// compre two C++ string case insensitive
int string_compare_ncase(const std::string & str1, const std::string & str2);

int string_compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2);


inline size_t copy_std_string_to_buff(char* buffer, size_t max_size, const std::string & str){
    const size_t str_size = str.copy(buffer, max_size-1);
    buffer[str_size]= '\0';
    return str_size;
}


#endif // STRINGUTILS_HPP
