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

#include <davix_internal.hpp>
#include <davixcontext.hpp>

#include <utils/stringutils.hpp>
#include <utils/davix_uri.hpp>
#include <modules/modules_profiles.hpp>
#include <backend/SessionFactory.hpp>
#include <davix_context_internal.hpp>
#include <core/RedirectionResolver.hpp>

#include <curl/curl.h>

#include <set>
#include <mutex>

namespace Davix{

struct LibPath;


static LibPath lib_path;


static bool redirCachingDisabled(){
    return ( getenv("DAVIX_DISABLE_REDIRECT_CACHING") != NULL);
}

///  Implementation f the core logic in davix
struct ContextInternal
{
    ContextInternal():
        _fsess(new SessionFactory()),
        _redirectionResolver(new RedirectionResolver(!redirCachingDisabled())),
        _hook_list()
    {
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "libdavix path {}, version: {}", getLibPath(), version());
    }

    ContextInternal(const ContextInternal & orig) :
        _fsess(new SessionFactory()),
        _redirectionResolver(new RedirectionResolver(!redirCachingDisabled())),
        _hook_list(orig._hook_list)
    {
    }

    virtual ~ContextInternal(){}

    // implementation of getSessionFactory
    inline SessionFactory* getSessionFactory(){
         return _fsess.get();
    }

    inline RedirectionResolver* getRedirectionResolver() {
        return _redirectionResolver.get();
    }

    std::unique_ptr<SessionFactory>  _fsess;
    std::unique_ptr<RedirectionResolver> _redirectionResolver;
    HookList _hook_list;
};

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


Context::Context() :
    _intern(new ContextInternal())
{
}

Context::Context(const Context &c) :
    _intern(new ContextInternal(*(c._intern))){
}

Context & Context::operator=(const Context & c){
    if( this != &c ){
        if( _intern != NULL)
            delete _intern;
        _intern = new ContextInternal(*(c._intern));
    }
    return *this;
}

Context::~Context(){
    delete _intern;
}

Context* Context::clone(){
    return new Context(*this);
}


void Context::setSessionCaching(bool caching){
    _intern->_fsess->setSessionCaching(caching);
}

bool Context::getSessionCaching() const{
    return _intern->_fsess->getSessionCaching();
}

void Context::clearCache() {
  _intern->_fsess.reset(new SessionFactory());
}

HttpRequest* Context::createRequest(const std::string & url, DavixError** err){
    return new HttpRequest(*this, Uri(url), err);
}


HttpRequest* Context::createRequest(const Uri &uri, DavixError **err){
    return new HttpRequest(*this, uri, err);
}


void Context::loadModule(const std::string &name){
    if( StrUtil::compare_ncase("grid",name) == 0){
        loadGridProfile(*this);
        return;
    }
    DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_CORE, "No module named {} found", name);
}

HookList & Context::getHookList(){
    return _intern->_hook_list;
}

SessionFactory& ContextExplorer::SessionFactoryFromContext(Context & c){
    return *(c._intern->getSessionFactory());
}

RedirectionResolver & ContextExplorer::RedirectionResolverFromContext(Context &c) {
    return *c._intern->getRedirectionResolver();
}

LibPath::LibPath(){
    Dl_info shared_lib_infos;

    // do an address resolution on a local function
    // get this resolution to determine davix shared library path at runtime
    if( dladdr((void*) &version, &shared_lib_infos) != 0){
        path = shared_lib_infos.dli_fname;
    }

}

const std::string & version(){
    static const std::string _version = DAVIX_VERSION_STRING;
    return _version;
}

const std::string backendRuntimeVersion() {
    curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

    std::ostringstream ss;
    ss << data->version << " " << data->ssl_version;

    return ss.str();
}

const std::string backendHeadersVersion() {
    std::ostringstream ss;
    ss << LIBCURL_VERSION;

    return ss.str();
}


const std::string & getLibPath(){
    return lib_path.path;
}


} // End Davix
