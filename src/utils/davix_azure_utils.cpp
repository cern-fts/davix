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

#include <iomanip>
#include <utils/davix_azure_utils.hpp>
#include "libs/datetime/datetime_utils.hpp"
#include <davix_internal.hpp>
#include <utils/davix_logger_internal.hpp>
#include "libs/alibxx/crypto/hmacsha.hpp"
#include "libs/alibxx/crypto/base64.hpp"

#define AZURE_TIME_LEEWAY 300

namespace Davix {
namespace Azure {

std::string extract_azure_account(const Uri & url) {
    std::string host = url.getHost();
    return host.substr(0, host.find("."));
}

std::string extract_azure_container(const Uri & url) {
    std::string path = url.getPath();
    std::string name = path.substr(1, path.find("/", 1));
    if(name.compare(name.size()-1,1,"/") == 0) {
        name.erase(name.size()-1, name.size());
    }
    return name;
}

std::string extract_azure_filename(const Uri & url) {
    std::string path = url.getPath();
    std::size_t sep = path.find("/", 1);
    if(sep == std::string::npos) return "";
    return path.substr(path.find("/", 1)+1, path.size());
}

std::string hexEncode(std::string input, std::string separator="") {
    std::ostringstream ss;
    for(std::string::iterator it = input.begin(); it != input.end(); it++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned int) ( (unsigned char) *it) << separator;
    }
    return ss.str();
}

Uri transformURI(const Uri & original_url, const RequestParams & params, const bool addDelimiter) {
    Uri newUri = original_url;
    newUri.setPath("/" + extract_azure_container(original_url) + "/");
    newUri.addQueryParam("restype", "container");
    newUri.addQueryParam("comp", "list");

    std::string filename = extract_azure_filename(original_url);
    if(filename[filename.size()-1] != '/') {
        filename.append("/");
    }

    // special case: listing top-dir
    if(filename == "/")
        filename = "";

    newUri.addQueryParam("prefix", filename);
    newUri.addQueryParam("delimiter", "/");

    return newUri;
}

// wrapper function, try to figure out what resourceType and permissions to use,
// based on the URL and the method.
// Try to always give as restrictive permissions as possible.
Uri signURI(const AzureSecretKey key, const std::string method, const Uri & url, const time_t signDuration) {
    if(method == "DELETE") {
        return signURI(key, Azure::Resource::BLOB, Azure::Permission::DELETE, url, signDuration);
    }
    else if(method == "PUT") {
        return signURI(key, Azure::Resource::BLOB, Azure::Permission::WRITE, url, signDuration);
    }
    else if(method == "GET") {
        const std::string filename = extract_azure_filename(url);
        if(filename.size() == 0) {
            return signURI(key, Azure::Resource::CONTAINER, Azure::Permission::LIST, url, signDuration);
        }
        return signURI(key, Azure::Resource::BLOB, Azure::Permission::READ, url, signDuration);
    }
    else if(method == "HEAD") {
        return signURI(key, Azure::Resource::BLOB, Azure::Permission::READ, url, signDuration);
    }
    throw std::runtime_error("unsupported method given to azure");
}

Uri signURI(const AzureSecretKey key, const Azure::Resource::Type resourceType, const Azure::Permission::Type permissions, const Uri & url,
            const time_t signDuration) {
    // reference: https://msdn.microsoft.com/en-us/library/azure/dn140255.aspx
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_S3, "Signing Azure URL");

    // build string to sign
    const std::string format("%Y-%m-%dT%H:%M:%SZ");
    time_t present = time(NULL);

    std::string signedpermissions = permissions;
    if(permissions == Azure::Permission::WRITE) signedpermissions = "acw";
    std::string signedstart = time_as_string(present-AZURE_TIME_LEEWAY, format);
    std::string signedexpiry = time_as_string(present+signDuration, format);

    std::string canonicalizedresource;
    if(resourceType == Resource::CONTAINER) {
        canonicalizedresource = "/blob/" + extract_azure_account(url) + "/" + extract_azure_container(url);
    } else if(resourceType == Resource::BLOB) {
        canonicalizedresource = "/blob/" + extract_azure_account(url) + url.getPath();
    }

    std::string signedidentifier;
    std::string signedIP;
    std::string signedProtocol;
    std::string signedversion("2016-05-31");
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
    std::string decoded_key = Base64::base64_decode(key);
    std::string signature = hmac_sha256(decoded_key, stringToSign.str());
    std::string base64sig = Base64::base64_encode((unsigned char*) signature.c_str(), signature.size());
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "signature: {}", base64sig);

    // add query parameters
    std::string signedresource;
    if(resourceType == Resource::CONTAINER) {
        signedresource = "c";
    } else if(resourceType == Resource::BLOB) {
        signedresource = "b";
    }

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
