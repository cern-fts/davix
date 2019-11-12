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
#include <core/SessionPool.hpp>

namespace Davix {

class HttpRequest;

struct NeonHandle {
    NeonHandle() : session(NULL) {}
    NeonHandle(const std::string &k, ne_session *s) : key(k), session(s) {}
    ~NeonHandle();

    std::string key;
    ne_session *session;
};

typedef std::shared_ptr<NeonHandle> NeonHandlePtr;

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
    void storeNeonSession(NeonHandlePtr sess);

    //--------------------------------------------------------------------------
    // Set caching on or off
    //--------------------------------------------------------------------------
    void setSessionCaching(bool caching);

    //--------------------------------------------------------------------------
    // Get caching status
    //--------------------------------------------------------------------------
    bool getSessionCaching() const;

private:
    //--------------------------------------------------------------------------
    // Neon session pool
    //--------------------------------------------------------------------------
    SessionPool<NeonHandlePtr> _session_pool;

    NeonHandlePtr create_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);
    NeonHandlePtr create_recycled_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);

    //--------------------------------------------------------------------------
    // Create a brand new neon session object, internal use only.
    //--------------------------------------------------------------------------
    NeonHandlePtr createNeonSession(const RequestParams & params, const Uri & uri, DavixError** err);

    //--------------------------------------------------------------------------
    // Variables to control session caching
    //--------------------------------------------------------------------------
    mutable std::mutex _session_caching_mtx;
    bool _session_caching;

};

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
