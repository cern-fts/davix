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

#ifndef DAVIX_DELEGATION_HPP
#define DAVIX_DELEGATION_HPP

#include <davix.hpp>
#include <string>

namespace Davix {

extern const std::string DELEGATION_SCOPE;

// Need a class so it can be frieds with Session
class DavixDelegation {
public:

    // Try out all the given delegation endpoints, until one works. Error is reported
    // iff all of them fail.
    static std::string delegate(Context & context, const std::vector<std::string> &dlg_endpoint,
        const Davix::RequestParams& params, Davix::DavixError** err);

    static std::string delegate(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, Davix::DavixError** err);

private:
    static bool get_credentials(const RequestParams& params,
            std::string& ucred, std::string& passwd, std::string& capath,
            int *lifetime, DavixError** err);

    static std::string delegate_v1(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, const std::string& ucred, const std::string& passwd,
            const std::string& capath,
            int lifetime, Davix::DavixError** err);

    static std::string delegate_v2(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, const std::string& ucred, const std::string& passwd,
            const std::string& capath,
            int lifetime, Davix::DavixError** err);
};

}

#endif
