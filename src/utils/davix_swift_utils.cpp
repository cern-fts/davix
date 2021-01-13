/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Shiting Long <s.long@fz-juelich.de>
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

Uri signURI(const RequestParams & params, const std::string & method, const Uri & url, HeaderVec headers, const time_t expirationTime) {
    Uri signed_url(url);

    if(!params.getSwiftProjectID().empty()) {
        signed_url.setPath("/v1/AUTH_" + params.getSwiftProjectID() + url.getPath());
    }
    return signed_url;
}

}

}
