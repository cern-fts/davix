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

#include <string>
#include <algorithm>
#include <davix_internal.hpp>
#include "neonsessionfactory.hpp"

#include <utils/davix_logger_internal.hpp>

namespace Davix {

static boost::once_flag neon_once = BOOST_ONCE_INIT;

static void init_neon(){
    ne_sock_init();
}

static bool sessionCachingDisabled(){
    return ( getenv("DAVIX_DISABLE_SESSION_CACHING") != NULL);
}

static bool redirCachingDisabled(){
    return ( getenv("DAVIX_DISABLE_REDIRECT_CACHING") != NULL);
}


NEONSessionFactory::NEONSessionFactory() :
    _sess_map(),
    _sess_mut(),
    _session_caching(!sessionCachingDisabled()),
    _redir_caching(!redirCachingDisabled()),
    _redirCache(256)
{
    boost::call_once(&init_neon, neon_once);
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "HTTP/SSL Session caching {}", (_session_caching?"ENABLED":"DISABLED"));
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Redirection Session caching {}", (_redir_caching?"ENABLED":"DISABLED"));
}

NEONSessionFactory::~NEONSessionFactory(){
    boost::lock_guard<boost::mutex> lock(_sess_mut);
    for(std::multimap<std::string, ne_session*>::iterator it = _sess_map.begin(); it != _sess_map.end(); ++it){
        ne_session_destroy(it->second);
    }
}

inline std::string davix_session_uri_rewrite(const Uri & u){
    std::string proto = u.getProtocol();
    if(proto.compare(0,4, "http") ==0
            || proto.compare(0,2, "s3") == 0
            || proto.compare(0,3, "dav") == 0){
        proto.assign("http");
        if(*(u.getProtocol().rbegin()) == 's')
            proto.append("s");
        return proto;
    }
    return std::string();
}

int NEONSessionFactory::createNeonSession(const RequestParams & params, const Uri & uri, ne_session** sess, DavixError **err){
    if(uri.getStatus() == StatusCode::OK){
        if(sess != NULL){
            std::string scheme = davix_session_uri_rewrite(uri);
            if(scheme.size() > 0){
                *sess = create_recycled_session(params, scheme, uri.getHost(), httpUriGetPort(uri));
                return 0;
            }
        }
    }

    DavixError::setupError(err, davix_scope_http_request(), StatusCode::UriParsingError, fmt::format("impossible to parse {}, not a valid HTTP, S3 or Webdav URL", uri.getString()));
    return -1;
}

int NEONSessionFactory::storeNeonSession(ne_session* sess){
    internal_release_session_handle(sess);
    return 0;
}




ne_session* NEONSessionFactory::create_session(const RequestParams & params, const std::string & protocol, const std::string &host, unsigned int port){
    ne_session *se;
    se = ne_session_create(protocol.c_str(), host.c_str(), (int) port);

    const Uri* proxy = params.getProxyServer();
    if(se != NULL && proxy != NULL){
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, " configure mandatory proxy to {}", proxy->getString().c_str());
        const enum ne_sock_sversion version = ((proxy->getProtocol().compare("socks5") ==0)?NE_SOCK_SOCKSV5:NE_SOCK_SOCKSV4);
        const int port_proxy = ((proxy->getPort() ==0)?1080:(proxy->getPort()));
        const std::string & userinfo = proxy->getUserInfo();
        std::string user, password;
        std::string::const_iterator delimiter = std::find(userinfo.begin(), userinfo.end(), ':');

        if(delimiter != userinfo.end()){
            user.assign(std::string(userinfo.begin(), delimiter));
            password.assign((delimiter+1), userinfo.end());
            ne_session_socks_proxy(se, version, proxy->getHost().c_str(), port_proxy, user.c_str(), password.c_str());
        }else{
            ne_session_socks_proxy(se, version, proxy->getHost().c_str(),  port_proxy, NULL, NULL);
        }

    }
    //ne_ssl_trust_default_ca(se); not stable in neon on epel 5
    return se;
}

ne_session* NEONSessionFactory::create_recycled_session(const RequestParams & params, const std::string &protocol, const std::string &host, unsigned int port){

    if(params.getKeepAlive()){
        ne_session* se= NULL;
        boost::lock_guard<boost::mutex> lock(_sess_mut);
        std::multimap<std::string, ne_session*>::iterator it;
        if( (it = _sess_map.find(create_map_keys_from_URL(protocol, host, port))) != _sess_map.end()){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "cached ne_session found ! taken from cache ");
            se = it->second;
            _sess_map.erase(it);
            return se;
        }

    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "no cached ne_session, create a new one ");
    return create_session(params, protocol, host, port);
}

void NEONSessionFactory::setSessionCaching(bool caching){
    _session_caching = caching && !sessionCachingDisabled();
}

void NEONSessionFactory::internal_release_session_handle(ne_session* sess){
    // clear sensitive data
    // none
    //
    boost::lock_guard<boost::mutex> lock(_sess_mut);
    std::multimap<std::string, ne_session*>::iterator it;
    std::string sess_key;
    sess_key.append(ne_get_scheme(sess)).append(ne_get_server_hostport(sess));

    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "add old session to cache {}", sess_key.c_str());

    _sess_map.insert(std::pair<std::string, ne_session*>(sess_key, sess));
}

static const std::pair<std::string, std::string> redirectionCreateKey(const std::string & method, const Uri & origin){
    std::string mymethod = method;
    // cache HEAD and GET on same key
    if(mymethod == "HEAD")
        mymethod = "GET";

    return std::make_pair(origin.getString(), mymethod);
}

void NEONSessionFactory::addRedirection( const std::string & method, const Uri & origin, boost::shared_ptr<Uri> dest){
    if(_redir_caching){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Add cached redirection <{} {} {}>", method.c_str(), origin.getString().c_str(), dest->getString().c_str());
        _redirCache.insert(redirectionCreateKey(method, origin), dest);
    }
}

boost::shared_ptr<Uri> NEONSessionFactory::redirectionResolve(const std::string & method, const Uri & origin){
    boost::shared_ptr<Uri> res = redirectionResolveSingle(method, origin);
    if(res.get() != NULL){
        boost::shared_ptr<Uri> res_rec = redirectionResolve(method, *res);
        if(res_rec.get() != NULL)
            return res_rec;
    }
    return res;
}

boost::shared_ptr<Uri> NEONSessionFactory::redirectionResolveSingleIntern(const std::string & method, const Uri & origin){
    return  _redirCache.find(redirectionCreateKey(method, origin));
}


boost::shared_ptr<Uri> NEONSessionFactory::redirectionResolveSingle(const std::string & method, const Uri & origin){
    boost::shared_ptr<Uri> res = redirectionResolveSingleIntern(method, origin);
    if(res.get() != NULL){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Found redirection  <{} {} {}>", method.c_str(), origin.getString().c_str(), res->getString().c_str());
    }
    return res;
}

void NEONSessionFactory::redirectionClean(const std::string & method, const Uri & origin){
    boost::shared_ptr<Uri> res = redirectionResolveSingleIntern(method, origin);
    if(res.get() != NULL){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Delete Cached redirection for <{} {} {}>", method.c_str(), origin.getString().c_str(), res->getString().c_str());
        _redirCache.erase(redirectionCreateKey(method, origin));
        redirectionClean(method, *res);
    }
}

void NEONSessionFactory::redirectionClean(const Uri & origin){
    std::pair<std::string, std::string> query = std::make_pair(origin.getString(), "");
    while(1) {
        const std::pair<std::string, std::string> nextkey = _redirCache.upper_bound(query);
        if(nextkey.first != origin.getString())
            break;

        redirectionClean(nextkey.second, nextkey.first);
    }
}

std::string create_map_keys_from_URL(const std::string & protocol, const std::string &host, unsigned int port){
    std::string host_port;
    if( (strcmp(protocol.c_str(), "http") ==0 && port == 80)
            || ( strcmp(protocol.c_str(), "https") ==0 && port == 443)){
        host_port = fmt::format("{}{}", protocol, host);
    }else
        host_port = fmt::format("{}{}:{}", protocol, host, port);
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, " creating session keys... {}", host_port);
    return host_port;
}

} // namespace Davix
