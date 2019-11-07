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

#ifndef DAVIX_NEONSESSIONFACTORY_H
#define DAVIX_NEONSESSIONFACTORY_H

#include <map>
#include <mutex>
#include <utils/davix_uri.hpp>
#include <neon/neonrequest.hpp>

namespace Davix {

using ne_session_ptr = std::unique_ptr<ne_session, decltype(&ne_session_destroy)>;

class HttpRequest;

class NEONSessionFactory
{
public:
    NEONSessionFactory();
    virtual ~NEONSessionFactory();

    //--------------------------------------------------------------------------
    // Create a NEONSession tied to this class.
    //--------------------------------------------------------------------------
    std::unique_ptr<NEONSession> provideNEONSession(const Uri &uri, const RequestParams &params, DavixError **err);

    //--------------------------------------------------------------------------
    // Store a Neon session object for session reuse purposes
    //--------------------------------------------------------------------------
    void storeNeonSession(ne_session_ptr sess);

    //--------------------------------------------------------------------------
    // Set caching on or off
    //--------------------------------------------------------------------------
    void setSessionCaching(bool caching);

    //--------------------------------------------------------------------------
    // Get caching status
    //--------------------------------------------------------------------------
    bool getSessionCaching() const;

private:
    // session pool
    std::multimap<std::string, ne_session_ptr> _sess_map;
    std::mutex _sess_mut;

    void internal_release_session_handle(ne_session_ptr sess);
    ne_session_ptr create_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);
    ne_session_ptr create_recycled_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);

    //--------------------------------------------------------------------------
    // Create a brand new neon session object, internal use only.
    //--------------------------------------------------------------------------
    ne_session_ptr createNeonSession(const RequestParams & params, const Uri & uri, DavixError** err);

    //--------------------------------------------------------------------------
    // Variables to control session caching
    //--------------------------------------------------------------------------
    mutable std::mutex _session_caching_mtx;
    bool _session_caching;

};

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
