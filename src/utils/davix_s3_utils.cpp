#include <utils/davix_s3_utils.hpp>

#include <ctime>
#include <cstring>
#include <davix_internal.hpp>
#include <string_utils/stringutils.hpp>
#include <base64/base64.hpp>
#include <hmac_sha1/hmacsha1.hpp>


namespace Davix{


std::string getAwsReqToken(const std::string & stringToSign, const std::string & private_key, const std::string & access_key){
    std::ostringstream ss;
    const std::string hmac = hmac_sha1(private_key, stringToSign);
    ss << Base64::base64_encode((unsigned char*) hmac.c_str(), hmac.size());
    return Uri::escapeString(ss.str());
}

std::string getAwsAuthorizationField(const std::string & stringToSign, const std::string & private_key, const std::string & access_key){
    std::ostringstream ss;
    const std::string hmac = hmac_sha1(private_key, stringToSign);
    ss << "AWS "<< access_key << ":" << Base64::base64_encode((unsigned char*) hmac.c_str(), hmac.size());
    return ss.str();
}


namespace S3{


static std::string extract_bucket(const Uri & uri){
    const std::string & hostname = uri.getHost();
    std::string::const_iterator it = std::find(hostname.begin(), hostname.end(),'.');
    return std::string(hostname.begin(), it);
}


static std::string get_date(HeaderVec & vec){
    for(HeaderVec::iterator it = vec.begin(); it < vec.end(); it++){
        if( StrUtil::compare_ncase(it->first, "date") ==0){
            return it->second;
        }
    }
    // create date

    struct tm utc_current;
    time_t t = time(NULL);
    char date[255];

    date[254]= '\0';
#ifdef HAVE_GMTIME_R
    gmtime_r(&t, &utc_current);
#else
    struct tm* p_utc = gmtime(&t);
    memcpy(&utc_current, p_utc, sizeof(struct tm));
#endif
    strftime(date, 254, "%a, %d %b %Y %H:%M:%S %z", &utc_current);

    // push date
    vec.push_back(std::pair<std::string, std::string>("Date", date));
    return std::string(date);
}


void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers){
    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << "\n"          // TODO : implement Content-type and md5 parser
       << "\n"
       << get_date(headers) << "\n"
       << '/' << extract_bucket(url)  << url.getPath();
    headers.push_back(std::pair<std::string, std::string>("Authorization",  getAwsAuthorizationField(ss.str(), params.getAwsAutorizationKeys().first, params.getAwsAutorizationKeys().second)));
}


Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime){
    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << "\n"          // TODO : implement Content-type and md5 parser
       << "\n"
       << static_cast<unsigned long long>(expirationTime) << "\n"
       << '/' << extract_bucket(url)  << url.getPath();
    const std::string signature = getAwsReqToken(ss.str(), params.getAwsAutorizationKeys().first, params.getAwsAutorizationKeys().second);


    ss.clear();
    ss.str("");
    ss << url.getString();
    if(url.getQuery().size() ==0){
        ss << "?";
    }else{
        ss << "&";
    }
    ss << "AWSAccessKeyId=" << params.getAwsAutorizationKeys().second << "&";
    ss << "Signature=" << signature << "&";
    ss << "Expires=" << static_cast<unsigned long long>(expirationTime);
    return Uri(ss.str());
}


} // S3


} // Davix
