/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2021
 * Author: Shiting Long <s.long@fz-juelich.de> (Forschungszentrum Juelich)
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

#include <utils/davix_swift_utils.hpp>
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

namespace Davix {

namespace Swift {

std::string extract_swift_path(const Uri & uri) {
    const std::string path = uri.getPath();

    std::size_t pos = path.find("/", 1);
    if(pos == std::string::npos) {
        return std::string("/");
    }

    return path.substr(pos, path.size());
}

std::string extract_swift_container(const Uri & uri) {
    const std::string path = uri.getPath();
    std::size_t pos = path.find("/", 1);
    if(pos == std::string::npos) {
        return path.substr(1, path.size()-1);
    }
    return path.substr(1, pos-1);
}

Uri signURI(const RequestParams & params, const Uri & url) {
    Uri signed_url(url);

    if(!params.getSwiftAccount().empty()) {
        signed_url.setPath("/v1/" + params.getSwiftAccount() + url.getPath());
    }
    else if (!params.getOSProjectID().empty()) {
        signed_url.setPath("/v1/AUTH_" + params.getOSProjectID() + url.getPath());
    }
    return signed_url;
}

Uri swiftUriTransformer(const Uri & original_url, const RequestParams & params, const bool addDelimiter) {
    std::string delimiter = "&delimiter=%2F";
    std::string prefix = "?prefix=";

    std::string protocol;

    const std::string url_string = original_url.getString();
    std::size_t pos = url_string.find(':');

    if(url_string.compare(pos-1,1,"s") == 0){
        protocol = "swifts://";
    }
    else{
        protocol = "swift://";
    }

    std::ostringstream ss;

    ss << protocol << original_url.getHost();
    if(original_url.getPort() > 0) {
        ss << ":" << original_url.getPort();
    }
    ss << "/";

    std::string container = extract_swift_container(original_url);
    if(container.size() != 0){
        ss << extract_swift_container(original_url) << "/";
    }

    if(!original_url.getPath().empty()){    // there is something after '/', grab it
        std::string tmp = extract_swift_path(original_url);

        // if prefix doesn't end with '/', add one to handle query on folder
        if(tmp.compare(tmp.size()-1,1,"/") != 0)
            tmp += "/";

        tmp.erase(0,1);
        prefix +=  Uri::queryParamEscape(tmp);
    }

    ss << prefix;

    // skip delimiter if where we want to list everything after a certain prefix,
    // useful in cases like GET Collection
    if(addDelimiter)
        ss << delimiter;

    return Uri(ss.str());
}

}

}
