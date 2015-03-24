#include <utils/davix_s3_utils.hpp>

#include <ctime>
#include <cstring>
#include <davix_internal.hpp>
#include <string_utils/stringutils.hpp>
#include <alibxx/crypto/base64.hpp>
#include <alibxx/crypto/hmacsha1.hpp>


namespace Davix{


const std::string prefix_s3_header("x-amz-");
const std::string prefix_s3_date("x-amz-date");

std::string getAwsReqToken(const std::string & stringToSign, const std::string & private_key){
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


bool matchAmzheaders(const std::string & header_key){
    return (StrUtil::compare_ncase(header_key, prefix_s3_header, prefix_s3_header.size()) == 0
            && StrUtil::compare_ncase(header_key, prefix_s3_date) != 0);
}

std::string getAmzCanonHeaders(HeaderVec & headers){
    std::string canon_amz_headers;

    for(HeaderVec::iterator it = headers.begin(); it < headers.end(); ++it){
        std::string header_key = (*it).first, header_value = (*it).second;
        StrUtil::toLower(StrUtil::trim(header_key));
        StrUtil::toLower(StrUtil::trim(header_value));

        if( matchAmzheaders(header_key)){
            canon_amz_headers.reserve(canon_amz_headers.size() + header_key.size() + header_value.size() +1);
            canon_amz_headers += header_key;
            canon_amz_headers += ":";
            canon_amz_headers += header_value;
            canon_amz_headers += "\n";
        }
    }
    return canon_amz_headers;
}


void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers){
    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << "\n"          // TODO : implement Content-type and md5 parser
       << "\n"
       << get_date(headers) << "\n"
       << getAmzCanonHeaders(headers) << '/' << extract_bucket(url)  << url.getPath();

    headers.push_back(std::pair<std::string, std::string>("Authorization",  getAwsAuthorizationField(ss.str(), params.getAwsAutorizationKeys().first, params.getAwsAutorizationKeys().second)));
}


Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime){

    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << "\n"          // TODO : implement Content-type and md5 parser
       << "\n"
       << static_cast<unsigned long long>(expirationTime) << "\n"
       <<  getAmzCanonHeaders(headers) << '/' << extract_bucket(url)  << url.getPath();
    const std::string signature = getAwsReqToken(ss.str(), params.getAwsAutorizationKeys().first);


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

    // add amz headers as query parameters
    for(HeaderVec::iterator it = headers.begin(); it < headers.end(); ++it){
        if(matchAmzheaders(it->first)){
            std::string key= it->first, value = it->second;
            StrUtil::toLower(StrUtil::trim(key));
            StrUtil::toLower(StrUtil::trim(value));

            ss << "&"<< Uri::escapeString(key) << "=" << Uri::escapeString(value);
        }
    }
    return Uri(ss.str());
}


Uri s3UriTranslator(const Uri & original_url, const RequestParams & params, const bool addDelimiter){
    std::string delimiter = "&delimiter=/";
    std::string prefix = "?prefix=";
    std::string maxKey = "&max-keys=";

    std::ostringstream ss;

    ss << "s3://" << original_url.getHost() << "/";

    if(!original_url.getPath().empty()){    // there is something after '/', grab it
        std::string tmp = original_url.getPath();
        
        // if prefix doesn't end with '/', add one to handle query on folder
        if(tmp.compare(tmp.size()-1,1,"/") != 0)
             tmp += "/";
        
        tmp.erase(0,1); 
        prefix += tmp;
    }
    
    // skip delimiter if where we want to list everything after a certain prefix, 
    // useful in cases like GET Collection
    if(addDelimiter)
        ss << prefix << delimiter << maxKey << params.getS3MaxKey();    
    else 
        ss << prefix << maxKey << params.getS3MaxKey();

    return Uri(ss.str());
}

time_t s3TimeConverter(std::string &s3time){
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    size_t pos=0;
    std::string tmp;

    // check which datetime format is used
    if(strptime(s3time.c_str(), "%a, %d %b %Y %H:%M:%S %z", &t) == NULL){
        if((pos = s3time.find("T")) != std::string::npos){ // iso 8601
            tmp = s3time.substr(0,pos) + " " + s3time.substr(pos+1,s3time.find('.',pos)-1);
            strptime(tmp.c_str(), "%F %T", &t);
        }
    }
    time_t mtime = timegm(&t);
    return mtime;
}


} // S3


} // Davix
