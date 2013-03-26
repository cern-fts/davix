#ifndef HMACSHA1_HPP
#define HMACSHA1_HPP

#include <string>
#include <algorithm>

std::string hmac_sha1(const std::string & key, const std::string & data);


#endif // HMACSHA1_HPP
