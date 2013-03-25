#ifndef BASE64_HPP
#define BASE64_HPP

/*
   base64.cpp and base64.hpp

   Modified version from René Nyffenegger version

   Public Domain

*/

#include <string>

namespace Base64{

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);



}

#endif // BASE64_HPP
