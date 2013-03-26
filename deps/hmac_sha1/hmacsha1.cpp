#include <config.h>
#include "hmacsha1.hpp"


#ifdef HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/sha.h>

static std::string hmac_sha1_openssl(const std::string & key, const std::string & data){
    unsigned char buffer_res[41];
    unsigned int buffer_len= 40;
    HMAC(EVP_sha1(), (const unsigned char*)key.c_str(), key.size(), (const unsigned char*) data.c_str(), data.size(), buffer_res, &buffer_len);
    return std::string((char*) buffer_res, buffer_len);
}

#endif

std::string hmac_sha1(const std::string & key, const std::string & data){
#ifdef HAVE_OPENSSL
    return hmac_sha1_openssl(key, data);
#else
#error "No support for hmac calculation"
#endif
}




