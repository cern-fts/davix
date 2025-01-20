/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2025
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

#include <string>
#include <algorithm>
#include <optional>

#include "utils/davix_logger.hpp"
#include "utils/davix_env_variables.hpp"

namespace
{
    std::optional<bool> envVariableToFlag(const char* env_name) {
        char* env = std::getenv(env_name);

        if (env != nullptr) {
            std::string senv(env);
            std::transform(senv.begin(), senv.end(), senv.begin(), ::tolower);

            if (senv == "true" || senv == "yes" || senv == "y" || senv == "1") {
                return true;
            }

            if (senv == "false" || senv == "no" || senv == "n" || senv == "0") {
                return false;
            }
        }

        return std::nullopt;
    }
}

namespace EnvUtils {

/// Read the "DAVIX_USE_LIBCURL" environment variable
bool getUseLibCurlFlag() {
    auto res = envVariableToFlag("DAVIX_USE_LIBCURL");

    if (res.has_value()) {
        return *res;
    }

#ifdef LIBCURL_BACKEND_BY_DEFAULT
    return true;
#else
    return false;
#endif
}

/// Read the "DAVIX_DISABLE_SESSION_CACHING" environment variable
bool getDisableSessionCachingFlag() {
    auto res = envVariableToFlag("DAVIX_DISABLE_SESSION_CACHING");
    return res.has_value() ? res.value() : false;
}

/// Read the "DAVIX_DISABLE_REDIRECT_CACHING" environment variable
bool getDisableRedirectCachingFlag() {
    auto res = envVariableToFlag("DAVIX_DISABLE_REDIRECT_CACHING");
    return res.has_value() ? res.value() : false;
}

/// Read the "DAVIX_FORMAT_BEARER_TOKEN" environment variable
bool getFormatBearerTokenFlag() {
    auto res = envVariableToFlag("DAVIX_FORMAT_BEARER_TOKEN");
    return res.has_value() ? res.value() : false;
}

/// Read the "DAVIX_DISABLE_METALINK" environment variable
bool getDisableMetalinkFlag() {
    auto res = envVariableToFlag("DAVIX_DISABLE_METALINK");
    return res.has_value() ? res.value() : false;
}

/// Read the "DAVPOSIX_MPUPLOAD" environment variable
bool getMPUploadFlag() {
    auto res = envVariableToFlag("DAVPOSIX_MPUPLOAD");
    return res.has_value() ? res.value() : false;
}

/// Read the "DAVIX_PARTSIZE" environment variable
long getPartSizeValue() {
    auto env = std::getenv("DAVIX_PARTSIZE");

    if (env != nullptr) {
        char* endp;
        long val = strtol(env, &endp, 10);

        if (*endp == '\0') {
            return val;
        }
    }

    return -1;
}

/// Read the "DAVIX_DEBUG" environment variable
/// Allows to set the debug level via an envar (useful when Davix is used via a plugin)
int getTraceValue() {
    auto env = std::getenv("DAVIX_DEBUG");

    if (env != nullptr) {
        char dbgval = tolower(*env);

        if (dbgval == 'c') return DAVIX_LOG_CRITICAL;
        if (dbgval == 'w') return DAVIX_LOG_WARNING;
        if (dbgval == 'v') return DAVIX_LOG_VERBOSE;
        if (dbgval == 'd') return DAVIX_LOG_DEBUG;
        if (dbgval == 't') return DAVIX_LOG_TRACE;
        if (dbgval == 'a') return DAVIX_LOG_ALL;
    }

    return 0;
}

/// Read the "DAVIX_STAGING_AREA" environment variable
std::string getStagingAreaValue() {
    auto env = std::getenv("DAVIX_STAGING_AREA");
    return (env != nullptr) ? std::string(env) : "/tmp";
}

/// Read the "X509_CERT_DIR" environment variable
std::string getX509CertDirValue() {
    auto env = std::getenv("X509_CERT_DIR");
    return (env != nullptr) ? std::string(env) : "/etc/grid-security/certificates/";
}

/// Read the "X509_USER_PROXY" environment variable
std::string getX509UserProxyValue() {
    auto env = std::getenv("X509_USER_PROXY");
    return (env != nullptr) ? std::string(env) : "";
}

/// Read the "X509_USER_CERT" environment variable
std::string getX509UserCertValue() {
    auto env = std::getenv("X509_USER_CERT");
    return (env != nullptr) ? std::string(env) : "";

}

/// Read the "X509_USER_KEY" environment variable
std::string getX509UserKeyValue() {
    auto env = std::getenv("X509_USER_KEY");
    return (env != nullptr) ? std::string(env) : "";

}

} // EnvUtils
