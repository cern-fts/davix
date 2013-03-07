#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <vector>
#include <string>


// split a string following an array of delimiter ( similar to strtok )
std::vector<std::string> stringTokSplit(const std::string & str, const std::string & delimiter);

// compre two C++ string case insensitive
int string_compare_ncase(const std::string & str1, const std::string & str2);

int string_compare_ncase(const std::string & str1, off_t offset, size_t size, const char* cstr2);


#endif // STRINGUTILS_HPP
