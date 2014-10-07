#ifndef DAVIX_S3_UTILS_HPP
#define DAVIX_S3_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix{

namespace S3{


void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers);

Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime);

} // S3


} // Davix


#endif // DAVIX_S3_UTILS_HPP
