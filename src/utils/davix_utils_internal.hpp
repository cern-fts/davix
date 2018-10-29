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

#pragma once
#ifndef DAVIX_UTILS_INTERNAL_HPP
#define DAVIX_UTILS_INTERNAL_HPP

#include <set>
#include <utils/davix_uri.hpp>
#include <params/davixrequestparams.hpp>

namespace Davix {

inline bool isS3SignedURL(const Davix::Uri &url) {
    if(url.queryParamExists("AWSAccessKeyId") && url.queryParamExists("Signature")) {
    	return true;
    }

    if(url.queryParamExists("X-Amz-Credential") && url.queryParamExists("X-Amz-Signature")) {
    	return true;
    }

    return false;
}

inline std::string extractCanonicalizedResourceQueryParams(const Davix::Uri &url) {
    std::set<std::string> targetParams;

    targetParams.insert("acl");
    targetParams.insert("lifecycle");
    targetParams.insert("location");
    targetParams.insert("logging");
    targetParams.insert("notification");
    targetParams.insert("partNumber");
    targetParams.insert("policy");
    targetParams.insert("requestPayment");
    targetParams.insert("uploadId");
    targetParams.insert("uploads");
    targetParams.insert("versionId");
    targetParams.insert("versioning");
    targetParams.insert("versions");
    targetParams.insert("website");

    targetParams.insert("delete"); // ?

    ParamVec canonicalizedParams;

    const ParamVec &params = url.getQueryVec();
    for(ParamVec::const_iterator it = params.begin(); it != params.end(); it++) {
        if(targetParams.count(it->first) != 0u) {
          canonicalizedParams.push_back(*it);
        }
    }

    std::sort(canonicalizedParams.begin(), canonicalizedParams.end());

    if(canonicalizedParams.empty()) {
        return "";
    }

    std::ostringstream ss;
    ss << "?";

    for(ParamVec::const_iterator it = canonicalizedParams.begin(); it != canonicalizedParams.end(); it++) {
        ss << it->first;
        if(!it->second.empty()) {
          ss << "=" << it->second;
        }

        if(it+1 != canonicalizedParams.end()) {
            ss << "&";
        }
    }

    return ss.str();
}

}

#endif // DAVIX_UTILS_INTERNAL_HPP
