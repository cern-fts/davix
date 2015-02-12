#ifndef DAVIX_S3_UTILS_HPP
#define DAVIX_S3_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix{

namespace S3{


void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers);

Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime);

Uri s3UriTranslator(const Uri & original_url, const RequestParams & params);

time_t s3TimeConverter(std::string &s3time);

} // S3


} // Davix


#endif // DAVIX_S3_UTILS_HPP
