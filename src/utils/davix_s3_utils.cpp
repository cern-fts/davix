/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <utils/davix_s3_utils.hpp>

#include <iomanip>
#include <ctime>
#include <cstring>
#include <davix_internal.hpp>
#include <utils/stringutils.hpp>
#include "libs/datetime/datetime_utils.hpp"
#include <utils/davix_logger_internal.hpp>
#include <utils/davix_utils_internal.hpp>
#include "libs/alibxx/crypto/base64.hpp"
#include "libs/alibxx/crypto/hmacsha.hpp"
#include <openssl/md5.h>
#include <sys/mman.h>
#include <fstream>

template <typename T>
static inline std::string SSTR(const T& v)
{
    std::ostringstream stream;
    stream << v;
    return stream.str();
}


namespace Davix{


const std::string prefix_s3_header("x-amz-");
const std::string prefix_s3_date("x-amz-date");


std::string hexEncode(std::string input, std::string separator="") {
    std::ostringstream ss;
    for(std::string::iterator it = input.begin(); it != input.end(); it++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int) ( (unsigned char) *it) << separator;
    }
    return ss.str();
}

std::string getAwsReqToken(const std::string & stringToSign, const std::string & private_key){
    std::ostringstream ss;
    const std::string hmac = hmac_sha1(private_key, stringToSign);
    ss << Base64::base64_encode((unsigned char*) hmac.c_str(), hmac.size());
    return ss.str();
}

std::string getAwsAuthorizationField(const std::string & stringToSign, const std::string & private_key, const std::string & access_key){
    std::ostringstream ss;
    const std::string hmac = hmac_sha1(private_key, stringToSign);
    ss << "AWS "<< access_key << ":" << Base64::base64_encode((unsigned char*) hmac.c_str(), hmac.size());
    return ss.str();
}

std::string getAwsSignaturev4(const std::string & stringToSign, const std::string & private_key,
                              const std::string & region, const std::string & service,
                              const std::string & date) {
    const std::string kDate = hmac_sha256("AWS4" + private_key, date);
    const std::string kRegion = hmac_sha256(kDate, region);
    const std::string kService = hmac_sha256(kRegion, service);
    const std::string kSigning = hmac_sha256(kService, "aws4_request");
    return hexEncode(hmac_sha256(kSigning, stringToSign));
}

namespace S3{
std::string extract_s3_provider(const Uri & uri) {
    const std::string & hostname = uri.getHost();
    std::string::const_iterator it = std::find(hostname.begin(), hostname.end(),'.');
    return std::string(it, hostname.end());
}

std::string extract_s3_bucket(const Uri & uri, bool aws_alternate){
    if(!aws_alternate) {
        const std::string & hostname = uri.getHost();
        std::string::const_iterator it = std::find(hostname.begin(), hostname.end(),'.');
        return std::string(hostname.begin(), it);
    }
    else {
        const std::string path = uri.getPath();
        std::size_t pos = path.find("/", 1);
        if(pos == std::string::npos) {
            return path.substr(1, path.size()-1);
        }
        return path.substr(1, pos-1);
    }
}

std::string extract_s3_path(const Uri & uri, bool aws_alternate) {
    const std::string path = uri.getPath();
    if(!aws_alternate) return path;

    std::size_t pos = path.find("/", 1);
    if(pos == std::string::npos) {
        return std::string("/");
    }

    return path.substr(pos, path.size());
}

std::string detect_region(const Uri &uri) {
    DavixError *err = NULL;
    Context context;
    GetRequest req(context, uri, &err);

    if(err) {
        return "";
    }

    RequestParams p;
    p.setAwsRegion("null");
    p.setOperationRetry(0);

    req.setParameters(p);
    req.executeRequest(&err);

    DavixError::clearError(&err);

    std::string region;
    if(req.getAnswerHeader("x-amz-region", region)) {
        return region;
    }
    return "";
}

static std::string get_md5(const HeaderVec & vec){
    for(HeaderVec::const_iterator it = vec.begin(); it < vec.end(); it++){
        if( StrUtil::compare_ncase(it->first, "Content-MD5") ==0){
            return it->second;
        }
    }
    return "";
}

static std::string get_type(const HeaderVec & vec){
    for(HeaderVec::const_iterator it = vec.begin(); it < vec.end(); it++){
        if( StrUtil::compare_ncase(it->first, "Content-Type") ==0){
            return it->second;
        }
    }
    return "";
}

static std::string get_date(HeaderVec & vec){
    for(HeaderVec::iterator it = vec.begin(); it < vec.end(); it++){
        if( StrUtil::compare_ncase(it->first, "date") ==0){
            return it->second;
        }
    }

    std::string date = current_time("%a, %d %b %Y %H:%M:%S %z");

    // push date
    vec.push_back(std::pair<std::string, std::string>("Date", date));
    return date;
}


bool matchAmzheaders(const std::string & header_key){
    return (StrUtil::compare_ncase(header_key, prefix_s3_header, prefix_s3_header.size()) == 0
            && StrUtil::compare_ncase(header_key, prefix_s3_date) != 0);
}

HeaderVec getAmzCanonHeaders_vec(const HeaderVec & headers){
    HeaderVec canon_amz_headers;

    for(HeaderVec::const_iterator it = headers.begin(); it < headers.end(); ++it){
        std::string header_key = (*it).first, header_value = (*it).second;
        StrUtil::toLower(StrUtil::trim(header_key));
        StrUtil::trim(header_value);

        if( matchAmzheaders(header_key)){
            canon_amz_headers.push_back(*it);
        }
    }
    return canon_amz_headers;
}

std::string getAmzCanonHeaders(const HeaderVec & headers) {
    std::string canon_amz_headers;

    for(HeaderVec::const_iterator it = headers.begin(); it < headers.end(); ++it){
        std::string header_key = (*it).first, header_value = (*it).second;
        StrUtil::toLower(StrUtil::trim(header_key));
        StrUtil::trim(header_value);

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

void signRequestv2(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers){
    if(params.getAwsToken().size() != 0) {
        headers.push_back(HeaderLine("x-amz-security-token", params.getAwsToken()));
    }

    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << get_md5(headers) << "\n"
       << get_type(headers) << "\n"
       << get_date(headers) << "\n";

    ss << getAmzCanonHeaders(headers);

    // when using a path-based url, the bucket name is part of the path
    if(params.getAwsAlternate()) {
        ss << url.getPath();
    }
    else {
        ss << '/' << extract_s3_bucket(url) << url.getPath();
    }

    ss << extractCanonicalizedResourceQueryParams(url);

    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_S3, "String to sign (aws-alternate={}):  {}", params.getAwsAlternate(), StrUtil::stringReplace(ss.str(), "\n", "\\n"));
    headers.push_back(std::pair<std::string, std::string>("Authorization",  getAwsAuthorizationField(ss.str(), params.getAwsAutorizationKeys().first, params.getAwsAutorizationKeys().second)));
}


// Sign an S3 request by modifying the headers, not the URI
void signRequest(const RequestParams & params, const std::string & method,
                 const Uri & url, HeaderVec & headers){
    if(params.getAwsRegion().empty()) {
        signRequestv2(params, method, url, headers);
    }
    else {
        signRequestv4(params, method, url, headers);
    }
}

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

Uri signURIv4(const RequestParams & params, const std::string & method, const Uri & url, const HeaderVec headers, const time_t expirationTime) {
    // references
    // http://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-header-based-auth.html
    // http://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3, "Using S3 v4 signature authentication");

    // using a non-default port?
    bool defaultPort;
    std::string portPart;

    defaultPort = (url.getPort() == 0);
    defaultPort |= (url.getPort() == 80 && (url.getProtocol() == "http" || url.getProtocol() == "s3"));
    defaultPort |= (url.getPort() == 443 && (url.getProtocol() == "https" || url.getProtocol() == "s3s"));

    if(!defaultPort) portPart = SSTR(":" << url.getPort());

    // calculate canonical headers
    HeaderVec can_headers = getAmzCanonHeaders_vec(headers);
    can_headers.push_back(HeaderLine("Host", SSTR(url.getHost() << portPart)));
    std::sort(can_headers.begin(), can_headers.end());
    std::ostringstream can_headers_str;
    for(HeaderVec::iterator it = can_headers.begin(); it != can_headers.end(); it++) {
        can_headers_str << StrUtil::toLower(it->first) << ":" << StrUtil::trim(it->second) << "\n";
    }

    std::ostringstream signed_headers;
    for(HeaderVec::iterator it = can_headers.begin(); it != can_headers.end(); it++) {
        signed_headers << StrUtil::toLower(it->first);
        if(it+1 != can_headers.end()) {
            signed_headers << ";";
        }
    }

    // calculate query parameters
    ParamVec query_params;
    query_params.push_back(HeaderLine("X-Amz-Algorithm", "AWS4-HMAC-SHA256"));

    std::ostringstream credential;
    credential << params.getAwsAutorizationKeys().second
               << "/" << current_time("%Y%m%d")
               << "/" << params.getAwsRegion()
               << "/" << "s3"
               << "/" << "aws4_request";

    query_params.push_back(HeaderLine("X-Amz-Credential", credential.str()));

    if(params.getAwsToken().size() != 0) {
        query_params.push_back(HeaderLine("X-Amz-Security-Token", params.getAwsToken()));
    }

    // calculate amz date
    std::string amzdate = current_time("%Y%m%dT%H%M%SZ");
    query_params.push_back(HeaderLine("X-Amz-Date", amzdate));

    // add timeout
    std::string expiration = SSTR(expirationTime);
    query_params.push_back(HeaderLine("X-Amz-Expires", expiration));

    // add amz signed headers
    query_params.push_back(HeaderLine("X-Amz-SignedHeaders", signed_headers.str()));

    // calculate *canonical* query string
    ParamVec can_query_params;
    for(ParamVec::iterator it = query_params.begin(); it != query_params.end(); it++) {
        can_query_params.push_back(ParamLine(Uri::queryParamEscape(it->first), Uri::queryParamEscape(it->second)));
    }

    const ParamVec existing_params = url.getQueryVec();
    for(ParamVec::const_iterator it = existing_params.begin(); it != existing_params.end(); it++) {
        can_query_params.push_back(ParamLine(it->first, it->second));
    }

    std::sort(can_query_params.begin(), can_query_params.end());

    std::ostringstream can_query_str;
    for(ParamVec::iterator it = can_query_params.begin(); it != can_query_params.end(); it++) {
        can_query_str << it->first << "=" << it->second;
        if(it+1 != can_query_params.end()) {
            can_query_str << "&";
        }
    }

    // calculate canonical request
    std::ostringstream canonical_request;
    canonical_request << method << "\n"
                      << url.getPath() << "\n"
                      << can_query_str.str() << "\n"
                      << can_headers_str.str() << "\n"
                      << signed_headers.str() << "\n"
                      << "UNSIGNED-PAYLOAD";

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Canonical request: {}", canonical_request.str());
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Canonical request bytes: {}", hexEncode(canonical_request.str(), " "));

    // calculate canonical request hash
    std::string can_req_hash = sha256(canonical_request.str().c_str());
    std::string encoded_hash = hexEncode(can_req_hash);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Canonical request hash: {}", encoded_hash);

    // calculate string to sign
    std::ostringstream stringToSign;
    stringToSign << "AWS4-HMAC-SHA256" << "\n"
                 << amzdate << "\n"
                 << current_time("%Y%m%d") << "/" << params.getAwsRegion() << "/s3/aws4_request" << "\n"
                 << encoded_hash;

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "String to sign (aws-alternate={}): {}", params.getAwsAlternate(), StrUtil::stringReplace(stringToSign.str(), "\n", "\\n"));
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "String to sign bytes: {}", hexEncode(stringToSign.str(), " "));

    // whew.. now calculate the final signature
    Uri signedUrl = url;

    std::string signature = getAwsSignaturev4(stringToSign.str(), params.getAwsAutorizationKeys().first,
                                              params.getAwsRegion(), "s3", current_time("%Y%m%d"));

    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3, "Signature: {}", signature);
    signedUrl.addQueryParam("X-Amz-Signature", signature);

    for(ParamVec::iterator it = query_params.begin(); it != query_params.end(); it++) {
        signedUrl.addQueryParam(it->first, it->second);
    }


    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Original URL: {}", url);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Signed URL: {}", signedUrl);
    return signedUrl;
}


void signRequestv4(const RequestParams & params, const std::string & method,
                   const Uri & url, HeaderVec & headers) {
    // references
    // https://docs.aws.amazon.com/AmazonS3/latest/developerguide/sigv4-auth-using-authorization-header.html
    // https://docs.aws.amazon.com/AmazonS3/latest/developerguide/sig-v4-header-based-auth.html
    // http://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3,
               "Using S3 v4 signature authentication (header mode)");

    // timestamps — respect a caller-supplied x-amz-date (test seam); otherwise
    // generate. datestamp is derived from whichever amzdate we end up using.
    std::string amzdate;
    for(const auto & h : headers) {
        if(StrUtil::compare_ncase(h.first, "x-amz-date") == 0) {
            amzdate = h.second;
            break;
        }
    }
    if(amzdate.empty()) {
        amzdate = current_time("%Y%m%dT%H%M%SZ");
        headers.push_back(HeaderLine("x-amz-date", amzdate));
    }
    // x-amz-date is YYYYMMDDTHHMMSSZ (16 chars); datestamp is the YYYYMMDD prefix.
    if(amzdate.size() < 16) {
        throw DavixException(std::string("S3::signRequestv4"), StatusCode::InvalidArgument,
                             "Malformed x-amz-date: " + amzdate);
    }
    const std::string datestamp = amzdate.substr(0, 8);

    // payload hash: for bodyless verbs sign SHA256("") rather than the
    // "UNSIGNED-PAYLOAD" literal — strict gateways (Copernicus gateway) 403 on a
    // signed UNSIGNED-PAYLOAD against an empty body. SHA256("") = e3b0c442...
    //
    // NOTE: streaming/multipart uploads (STREAMING-AWS4-HMAC-SHA256-PAYLOAD) are
    // not implemented; PUT/POST use UNSIGNED-PAYLOAD.
    static const std::string EMPTY_SHA256 =
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    const bool bodyless =
        (method == "GET" || method == "HEAD" || method == "DELETE");
    const std::string payload_hash =
        bodyless ? EMPTY_SHA256 : std::string("UNSIGNED-PAYLOAD");

    // add content-sha256 only if not already present (dedup)
    bool has_content_sha = false;
    for(const auto & h : headers) {
        if(StrUtil::compare_ncase(h.first, "x-amz-content-sha256") == 0) {
            has_content_sha = true;
            break;
        }
    }
    if(!has_content_sha) {
        headers.push_back(HeaderLine("x-amz-content-sha256", payload_hash));
    }
    if(params.getAwsToken().size() != 0) {
        headers.push_back(HeaderLine("x-amz-security-token", params.getAwsToken()));
    }

    // host (omit default port)
    bool defaultPort = (url.getPort() == 0)
        || (url.getPort() == 80  && (url.getProtocol() == "http"  || url.getProtocol() == "s3"))
        || (url.getPort() == 443 && (url.getProtocol() == "https" || url.getProtocol() == "s3s"));
    std::string host = url.getHost();
    if(!defaultPort) host += SSTR(":" << url.getPort());

    // canonical headers: getAmzCanonHeaders_vec returns the x-amz-* headers
    // (original-case) but NOT host or x-amz-date, so add those explicitly.
    // Also add any signed non-amz headers the caller set (e.g. Range). The dedup
    // below makes these explicit pushes safe even if a header is also present in
    // the vector returned by getAmzCanonHeaders_vec.
    HeaderVec can_headers = getAmzCanonHeaders_vec(headers);
    can_headers.push_back(HeaderLine("host", host));
    can_headers.push_back(HeaderLine("x-amz-date", amzdate));

    // include "range" if supplied (AWS example signs host;range;...sha256;date)
    for(const auto & h : headers) {
        if(StrUtil::compare_ncase(h.first, "range") == 0) {
            can_headers.push_back(HeaderLine("range", h.second));
        }
    }

    // lowercase keys, then sort, then dedup by key
    for(auto & h : can_headers) h.first = StrUtil::toLower(h.first);
    std::sort(can_headers.begin(), can_headers.end());
    // Dedup by header key (keeps first value). NOTE: we do not combine duplicate
    // signed headers into a comma-separated value as AWS canonicalization allows;
    // davix does not emit duplicate signed headers in practice.
    can_headers.erase(
        std::unique(can_headers.begin(), can_headers.end(),
                    [](const HeaderLine & a, const HeaderLine & b){
                        return a.first == b.first;
                    }),
        can_headers.end());

    std::ostringstream can_headers_str, signed_headers_str;
    for(auto it = can_headers.begin(); it != can_headers.end(); ++it) {
        can_headers_str << it->first << ":" << StrUtil::trim(it->second) << "\n";
        signed_headers_str << it->first;
        if(it + 1 != can_headers.end()) signed_headers_str << ";";
    }

    // canonical query string: getQueryVec() returns decoded values; pass them
    // through unescaped to match signURIv4 (do not double-encode).
    ParamVec can_query_params;
    const ParamVec existing_params = url.getQueryVec();
    for(const auto & p : existing_params) {
        can_query_params.push_back(ParamLine(p.first, p.second));
    }
    std::sort(can_query_params.begin(), can_query_params.end());

    std::ostringstream can_query_str;
    for(auto it = can_query_params.begin(); it != can_query_params.end(); ++it) {
        can_query_str << it->first << "=" << it->second;
        if(it + 1 != can_query_params.end()) can_query_str << "&";
    }

    // canonical request
    std::ostringstream canonical_request;
    canonical_request << method << "\n"
                      << url.getPath() << "\n"
                      << can_query_str.str() << "\n"
                      << can_headers_str.str() << "\n"
                      << signed_headers_str.str() << "\n"
                      << payload_hash;

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3,
               "Canonical request (header mode) length: {}",
               canonical_request.str().size());

    // string to sign
    const std::string can_req_hash = hexEncode(sha256(canonical_request.str().c_str()));
    std::ostringstream string_to_sign;
    string_to_sign << "AWS4-HMAC-SHA256" << "\n"
                   << amzdate << "\n"
                   << datestamp << "/" << params.getAwsRegion() << "/s3/aws4_request" << "\n"
                   << can_req_hash;

    // signature
    const std::string signature = getAwsSignaturev4(string_to_sign.str(), params.getAwsAutorizationKeys().first,
                                                    params.getAwsRegion(), "s3", datestamp);

    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3, "Signature: {}", signature);

    // Authorization header
    std::ostringstream auth;
    auth << "AWS4-HMAC-SHA256 "
         << "Credential=" << params.getAwsAutorizationKeys().second
                          << "/" << datestamp
                          << "/" << params.getAwsRegion()
                          << "/s3/aws4_request, "
         << "SignedHeaders=" << signed_headers_str.str() << ", "
         << "Signature=" << signature;

    headers.push_back(HeaderLine("Authorization", auth.str()));
}


// return a signed s3 URI, does not modify the headers
Uri signURI(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec headers, const time_t expirationTime) {
    if(params.getAwsRegion().empty()) {
        return tokenizeRequest(params, method, url, headers, std::time(0)+expirationTime);
    }
    else {
        return signURIv4(params, method, url, headers, expirationTime);
    }
}


Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime){
    if(params.getAwsToken().size() != 0) {
        headers.push_back(HeaderLine("x-amz-security-token", params.getAwsToken()));
    }

    std::ostringstream ss;

    // construct Request token
    ss << method << "\n"
       << get_md5(headers) << "\n"
       << get_type(headers) << "\n"
       << static_cast<unsigned long long>(expirationTime) << "\n";

    ss << getAmzCanonHeaders(headers);

    // when using a path-based url, the bucket name is part of the path
    if(params.getAwsAlternate()) {
        ss << url.getPath();
    }
    else {
        ss << '/' << extract_s3_bucket(url) << url.getPath();
    }

    ss << extractCanonicalizedResourceQueryParams(url);
    const std::string signature = getAwsReqToken(ss.str(), params.getAwsAutorizationKeys().first);

    Uri signedUri(url);

    signedUri.addQueryParam("AWSAccessKeyId", params.getAwsAutorizationKeys().second);
    signedUri.addQueryParam("Signature", signature);
    signedUri.addQueryParam("Expires", SSTR(expirationTime));

    // add amz headers as query parameters
    for(HeaderVec::const_iterator it = headers.begin(); it < headers.end(); ++it){
         if(matchAmzheaders(it->first)){
             std::string key= it->first, value = it->second;
             StrUtil::toLower(StrUtil::trim(key));
             StrUtil::trim(value);

             signedUri.addQueryParam(key, value);
         }
    }

    return signedUri;
}


Uri s3UriTransformer(const Uri & original_url, const RequestParams & params, const bool addDelimiter){
    std::string delimiter = "&delimiter=%2F";
    std::string prefix = "?prefix=";
    std::string maxKey = "&max-keys=";

    std::string protocol;

    const std::string url_string = original_url.getString();
    std::size_t pos = url_string.find(':');

    if(url_string.compare(pos-1,1,"s") == 0){
        protocol = "s3s://";
    }
    else{
        protocol = "s3://";
    }

    std::ostringstream ss;

    ss << protocol << original_url.getHost();
    if(original_url.getPort() > 0) {
      ss << ":" << original_url.getPort();
    }
    ss << "/";

    if(params.getAwsAlternate()) {
        ss << extract_s3_bucket(original_url, params.getAwsAlternate()) << "/";
    }

    if(!original_url.getPath().empty()){    // there is something after '/', grab it
        std::string tmp = extract_s3_path(original_url, params.getAwsAlternate());

        // if prefix doesn't end with '/', add one to handle query on folder
        if(tmp.compare(tmp.size()-1,1,"/") != 0)
             tmp += "/";

        tmp.erase(0,1);
        prefix +=  Uri::queryParamEscape(tmp);
    }

    ss << prefix << maxKey << params.getS3MaxKey();

    // skip delimiter if where we want to list everything after a certain prefix,
    // useful in cases like GET Collection
    if(addDelimiter)
        ss << delimiter;

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

// taken from dmlite
std::string hexPrinter(const unsigned char* data, dav_size_t nbytes){
    char buffer[nbytes * 2 + 1];
    char *p;

    p = buffer;
    for(dav_size_t offset = 0; offset < nbytes; ++offset, p+=2)
        sprintf(p, "%02x", data[offset]);
    *p = '\0';

    return std::string(buffer);
}

int calculateMD5(std::string &input, std::string &output){
    if(input.empty()) return -1;

    unsigned char result_buf[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)input.c_str(), input.size(), result_buf);

    output = Base64::base64_encode(result_buf, MD5_DIGEST_LENGTH);

    return output.empty() ? -1 : 0;
}

int calculateMD5(int fd, std::string &output){
    struct stat statbuf;
    if(fstat(fd, &statbuf) < 0)
        return -1;

    unsigned char result_buf[MD5_DIGEST_LENGTH];

    void* file_buf = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    MD5((unsigned char*)file_buf, statbuf.st_size, result_buf);
    munmap(file_buf, statbuf.st_size);

    output = Base64::base64_encode(result_buf, MD5_DIGEST_LENGTH);

    return output.empty() ? -1 : 0;
}

} // S3


} // Davix
