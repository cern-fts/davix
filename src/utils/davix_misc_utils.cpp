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

#include <davix_internal.hpp>
#include <utils/davix_misc_utils.hpp>

namespace Davix {

WebdavSupport::Type detect_webdav_support(Context & context, const RequestParams &params, const Uri & uri, DavixError** err) {
    HttpRequest req(context, uri, err);
    req.setParameters(params);
    req.setRequestMethod("OPTIONS");
    req.executeRequest(err);

    std::string allow;
    req.getAnswerHeader("Allow", allow);

    if(*err) {
        return WebdavSupport::UNKNOWN;
    }

    if(allow.find("PROPFIND") != std::string::npos || allow.find("MKCOL") != std::string::npos) {
        return WebdavSupport::YES;
    }

    return WebdavSupport::NO;
}

}
