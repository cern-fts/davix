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
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <alibxx/containers/cache.hpp>

namespace Davix {

class HttpRequest;

class NEONSessionFactory
{
    friend class NEONRequest;
public:
    NEONSessionFactory();
    virtual ~NEONSessionFactory();

    /**
      Create a session object or create a recycled  one ( session reuse )
    */
    int createNeonSession(const RequestParams & params, const Uri & uri, ne_session** sess, DavixError** err);

    /**
      store a Neon session object for session reuse purpose
    */
    int storeNeonSession(ne_session *sess);

    //
    // Redirecton caching
    //

    void addRedirection( const std::string & method, const Uri & origin, boost::shared_ptr<Uri> dest);

    // try to find cached redirection, resolve a full chain
    boost::shared_ptr<Uri> redirectionResolve(const std::string & method, const Uri & origin);
    // try to find a cached redirection, resolve only one element
    boost::shared_ptr<Uri> redirectionResolveSingle(const std::string & method, const Uri & origin);

    void redirectionClean(const std::string & method, const Uri & origin);
    void redirectionClean(const Uri & origin);

    //
    // opts
    //

    void setSessionCaching(bool caching);

    inline bool getSessionCaching() const {
        return _session_caching;
    }

private:
    // session pool
    std::multimap<std::string, ne_session*> _sess_map;
    std::mutex _sess_mut;
    bool _session_caching, _redir_caching;

    // redirection pool
    Cache<std::pair<std::string, std::string>, Uri> _redirCache;

    void internal_release_session_handle(ne_session* sess);
    ne_session* create_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);
    ne_session* create_recycled_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port);

    boost::shared_ptr<Uri> redirectionResolveSingleIntern(const std::string & method, const Uri & origin);

};

void parse_http_neon_url(const std::string & url, std::string & protocol,
                         std::string & host, std::string & path, unsigned long *port);

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port);

} // namespace Davix



#endif // DAVIX_NEONSESSIONFACTORY_H
