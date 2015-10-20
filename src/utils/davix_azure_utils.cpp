/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2015
 * Author: Georgios Bitzes <georgios.bitzes@cern.ch>
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

#include <utils/davix_azure_utils.hpp>
#include <datetime/datetime_utils.hpp>
#include <davix_internal.hpp>
#include <utils/davix_logger_internal.hpp>
#include <alibxx/crypto/hmacsha.hpp>
#include <alibxx/crypto/base64.hpp>

namespace Davix {
namespace Azure {

static std::string extract_container(const Uri & url) {
    std::string host = url.getHost();
    return host.substr(0, host.find("."));
}

std::string hexEncode(std::string input, std::string separator="") {
    std::ostringstream ss;
    for(std::string::iterator it = input.begin(); it != input.end(); it++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int) ( (unsigned char) *it) << separator;
    }
    return ss.str();
}

static std::string permissions_for_method(const std::string & method) {
    if(method == "GET")
        return "r";
    if(method == "PUT")
        return "w";
    if(method == "DELETE")
        return "d";

    return "r";
}

Uri signURI(const RequestParams & params, const std::string & method, const Uri & url, const HeaderVec headers, const time_t signDuration) {
    // reference: https://msdn.microsoft.com/en-us/library/azure/dn140255.aspx
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3, "Signing Azure URL");

    // build string to sign
    const std::string format("%Y-%m-%dT%H:%M:%SZ");
    time_t present = time(NULL);

    std::string signedpermissions = permissions_for_method(method);
    std::string signedstart = time_as_string(present, format);
    std::string signedexpiry = time_as_string(present+signDuration, format);
    std::string canonicalizedresource = "/blob/" + extract_container(url) + url.getPath();
    std::string signedidentifier;
    std::string signedIP;
    std::string signedProtocol;
    std::string signedversion("2015-04-05");
    std::string rscc;
    std::string rscd;
    std::string rsce;
    std::string rscl;
    std::string rsct;

    std::ostringstream stringToSign;
    stringToSign << signedpermissions << "\n"
                 << signedstart << "\n"
                 << signedexpiry << "\n"
                 << canonicalizedresource << "\n"
                 << signedidentifier << "\n"
                 << signedIP << "\n"
                 << signedProtocol << "\n"
                 << signedversion << "\n"
                 << rscc << "\n"
                 << rscd << "\n"
                 << rsce << "\n"
                 << rscl << "\n"
                 << rsct;

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "string to sign: \n{}", stringToSign.str());
    std::string decoded_key = Base64::base64_decode(params.getAzureKey());
    std::string signature = hmac_sha256(decoded_key, stringToSign.str()); // params.getAzureKey());
    std::string base64sig = Base64::base64_encode((unsigned char*) signature.c_str(), signature.size());
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "signature: {}", base64sig);

    // add query parameters
    std::string signedresource("b");

    Uri signedUrl(url);
    signedUrl.addQueryParam("sv", signedversion);
    signedUrl.addQueryParam("st", signedstart);
    signedUrl.addQueryParam("se", signedexpiry);
    signedUrl.addQueryParam("sr", signedresource);
    signedUrl.addQueryParam("sp", signedpermissions);
    signedUrl.addQueryParam("sig", base64sig);

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "signed url: {}", signedUrl);
    return signedUrl;
}

}
}
