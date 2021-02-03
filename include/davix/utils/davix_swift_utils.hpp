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

#ifndef DAVIX_DAVIX_SWIFT_UTILS_H
#define DAVIX_DAVIX_SWIFT_UTILS_H

#include <params/davixrequestparams.hpp>

namespace Davix {

namespace Swift {

Uri signURI(const RequestParams & params, const Uri & url);

Uri swiftUriTransformer(const Uri & original_url, const RequestParams & params, const bool addDelimiter);

std::string extract_swift_path(const Uri & uri);

std::string extract_swift_container(const Uri & uri);

} //Swift

} //Davix

#endif //DAVIX_DAVIX_SWIFT_UTILS_H
