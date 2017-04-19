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

#ifndef DAVIX_FILEUTILS_HPP
#define DAVIX_FILEUTILS_HPP

#include <davix.hpp>
#include <utils/davix_fileproperties.hpp>

namespace Davix {

extern const std::string ans_header_byte_range;
extern const std::string ans_header_content_type;
extern const std::string ans_header_multi_part_value;
extern const std::string ans_header_boundary_field;
extern const std::string ans_header_content_length;
extern const std::string req_header_byte_range;


// take a HTTP request status and convert file status to common errcode
int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err);

void check_file_status(HttpRequest & req, const std::string & scope);

// configure Range request
void setup_offset_request(HttpRequest* req, const dav_off_t *start_len, const dav_size_t *size_read, const dav_size_t number_ops);


typedef std::function<int (dav_off_t &, dav_off_t &)> OffsetCallback;

std::vector< std::pair<dav_size_t, std::string> > generateRangeHeaders(dav_size_t max_header_size, OffsetCallback & offset_provider);


} // namespace Davix

#endif // DAVIX_FILEUTILS_HPP
