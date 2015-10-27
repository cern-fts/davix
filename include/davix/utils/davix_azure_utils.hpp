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

#ifndef DAVIX_AZURE_UTILS_HPP
#define DAVIX_AZURE_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix {
namespace Azure {

namespace Resource {
typedef enum {
    CONTAINER,
    BLOB
} Type;
}

namespace Permission {
const std::string READ("r");
const std::string CREATE("c");
const std::string WRITE("w");
const std::string LIST("l");
const std::string DELETE("d");

typedef std::string Type;
}

std::string extract_azure_filename(const Uri & url);
std::string extract_azure_container(const Uri & url);
std::string extract_azure_account(const Uri & url);

Uri transformURI(const Uri & original_url, const RequestParams & params, const bool addDelimiter);

Uri signURI(const AzureSecretKey key, const std::string method, const Uri & url, const time_t signDuration);

Uri signURI(const AzureSecretKey key, const Azure::Resource::Type resourceType, const Azure::Permission::Type permissions, const Uri & url,
            const time_t signDuration);

} // Azure
} // Davix



#endif // DAVIX_AZURE_UTILS_HPP

