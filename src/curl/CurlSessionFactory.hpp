/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#ifndef DAVIX_CURL_SESSION_FACTORY_HPP
#define DAVIX_CURL_SESSION_FACTORY_HPP

#include "../backend/SessionFactory.hpp"
#include <status/DavixStatus.hpp>
#include <core/SessionPool.hpp>

namespace Davix {

struct CurlHandle;
typedef std::shared_ptr<CurlHandle> CurlHandlePtr;

class CurlSession;

class CurlSessionFactory {
public:
    //--------------------------------------------------------------------------
    // Constructor
    //--------------------------------------------------------------------------
    CurlSessionFactory();

    //--------------------------------------------------------------------------
    // Destructor
    //--------------------------------------------------------------------------
    virtual ~CurlSessionFactory();

    //--------------------------------------------------------------------------
    // Create a CurlSession tied to this class.
    //--------------------------------------------------------------------------
    std::unique_ptr<CurlSession> provideCurlSession(const Uri &uri, const RequestParams &params, Status &st);

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
    // Retrieve cached handle, if possible
    //--------------------------------------------------------------------------
    CurlHandlePtr getCachedHandle(const Uri &uri, const RequestParams &params);

    //--------------------------------------------------------------------------
    // Provide brand-new handle
    //--------------------------------------------------------------------------
    CurlHandlePtr makeNewHandle(const Uri &uri, const RequestParams &params);

    //--------------------------------------------------------------------------
    // Variables to control session caching
    //--------------------------------------------------------------------------
    mutable std::mutex _session_caching_mtx;
    bool _session_caching;

    //--------------------------------------------------------------------------
    // Session pool
    //--------------------------------------------------------------------------
    SessionPool<CurlHandlePtr> _session_pool;
};

}

#endif

