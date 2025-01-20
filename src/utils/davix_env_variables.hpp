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

#pragma once

#include <string>

/**
 * Concentrate Davix environment variables in the "EnvUtil" namespace.
 * Each environment variable should have a dedicated getter,
 * signaling clear intent to use this variable.
 *
 * The getter will return True/False (for flag variables) or the actual value.
 * The flag boolean will be interpreted case-insensitive:
 *   - true for "True", "Yes", "Y", "1" values
 *   - false otherwise
 */
namespace EnvUtils {

/// Read the "DAVIX_USE_LIBCURL" environment variable
bool getUseLibCurlFlag();

/// Read the "DAVIX_DISABLE_SESSION_CACHING" environment variable
bool getDisableSessionCachingFlag();

/// Read the "DAVIX_DISABLE_REDIRECT_CACHING" environment variable
bool getDisableRedirectCachingFlag();

/// Read the "DAVIX_FORMAT_BEARER_TOKEN" environment variable
bool getFormatBearerTokenFlag();

/// Read the "DAVIX_DISABLE_METALINK" environment variable
bool getDisableMetalinkFlag();

/// Read the "DAVPOSIX_MPUPLOAD" environment variable
bool getMPUploadFlag();

/// Read the "DAVIX_PARTSIZE" environment variable
long getPartSizeValue();

/// Read the "DAVIX_DEBUG" environment variable
int getTraceValue();

/// Read the "DAVIX_STAGING_AREA" environment variable
std::string getStagingAreaValue();

/// Read the "X509_CERT_DIR" environment variable
std::string getX509CertDirValue();
/// Read the "X509_USER_PROXY" environment variable
std::string getX509UserProxyValue();
/// Read the "X509_USER_CERT" environment variable
std::string getX509UserCertValue();
/// Read the "X509_USER_KEY" environment variable
std::string getX509UserKeyValue();

} // EnvUtils
