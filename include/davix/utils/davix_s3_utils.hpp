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

#ifndef DAVIX_S3_UTILS_HPP
#define DAVIX_S3_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix{

namespace S3{

Uri signURIv4(const RequestParams & params, const std::string & method, const Uri & url, const HeaderVec headers, const time_t expirationTime);
Uri signURI(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec headers, const time_t expirationTime);

void signRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers);

Uri tokenizeRequest(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec & headers, time_t expirationTime);

Uri s3UriTransformer(const Uri & original_url, const RequestParams & params, const bool addDelimiter);

time_t s3TimeConverter(std::string &s3time);

std::string hexPrinter(const unsigned char* data, dav_size_t nbytes);

// MD5 from string
int calculateMD5(std::string &input, std::string &output);

// MD5 from fd
int calculateMD5(int fd, std::string &output);

// extract bucket and path information from a bucket
std::string extract_s3_provider(const Uri & uri);
std::string extract_s3_bucket(const Uri & uri, bool aws_alternate=false);
std::string extract_s3_path(const Uri & uri, bool aws_alternate=false);

std::string detect_region(const Uri &uri);

} // S3


} // Davix


#endif // DAVIX_S3_UTILS_HPP
