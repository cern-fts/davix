#ifndef DAVIX_S3_UTILS_HPP
#define DAVIX_S3_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix{

namespace S3{


void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers);

Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime);

Uri s3UriTransformer(const Uri & original_url, const RequestParams & params, const bool addDelimiter);

time_t s3TimeConverter(std::string &s3time);

std::string hexPrinter(const unsigned char* data, dav_size_t nbytes);

// MD5 from string
int calculateMD5(std::string &input, std::string &output);

// MD5 from fd
int calculateMD5(int fd, std::string &output);

} // S3


} // Davix


#endif // DAVIX_S3_UTILS_HPP
