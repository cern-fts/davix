#include <davix_internal.hpp>
#include <auth/davixauth.hpp>
#include <base64/base64.hpp>
#include <hmac_sha1/hmacsha1.hpp>

namespace Davix{

std::string getAwsAuthorizationField(const std::string & stringToSign, const std::string & private_key, const std::string & access_key){
    std::ostringstream ss;
    const std::string hmac = hmac_sha1(private_key, stringToSign);
    ss << "AWS "<< access_key << ":" << Base64::base64_encode((unsigned char*) hmac.c_str(), hmac.size());
    return ss.str();
}

std::vector<std::string> v;

SessionInfo::SessionInfo() : data(NULL){

}

std::vector<std::string> & SessionInfo::getReadableDN() const{
    // TODO: implement proper DN mapping
    return v;
}

}
