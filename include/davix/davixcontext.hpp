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

#ifndef DAVIXCONTEXT_HPP
#define DAVIXCONTEXT_HPP

#include <string>
#include <status/davixstatusrequest.hpp>
#include <hooks/davix_hooks.hpp>
#include <utils/davix_uri.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


///
/// @file davixcontext.hpp
/// @author Devresse Adrien
///
///  Handle of Davix

namespace Davix{

struct ContextInternal;
struct ContextExplorer;
class HookList;
class HttpRequest;
class DavPosix;



/// @brief Main handle for Davix
///
/// Each new davix context contains its own session-reuse pool and set of parameters
/// a Context can execute multiple queries in parallels and is thread safe
class DAVIX_EXPORT Context
{
public:
    ///
    /// \brief Default constructor
    ///
    Context();
    ///
    /// \brief copy constructor
    /// \param c
    ///
    Context(const Context & c);
    ///
    /// \brief assignment operator
    /// \param c
    /// \return
    ///
    Context & operator=(const Context & c);
    ///
    /// \brief destructor
    ///
    virtual ~Context();

    /// clone this instance to a new context
    Context* clone();

#ifdef __DAVIX_HAS_STD_FUNCTION

    /// set a new hook (callback) to intercept event in davix
    /// see davix_hooks.hpp for more details about hooks
    /// WARNING: setting a new HOOK override exiting ones
    template<typename HookType>
    inline void setHook(const HookType & id){
        hookDefine<HookType>(getHookList(), id);
    }

    /// get the value register for one type of hook
    template<typename HookType>
    inline const HookType & getHook(){
        return hookGet<HookType>(getHookList());
    }

#endif


    /// @brief load a plugin or a profile identified by name
    /// @param name : name of the plugin or  profile to load
    ///
    /// Example: loadModule("grid") configure davix
    /// for a grid environment usage
    void loadModule(const std::string & name);

    ///  enable or disable the session caching
    void setSessionCaching(bool caching);

    /// get session caching status
    bool getSessionCaching() const;

    /// clear both redirect and session cache
    void clearCache();

private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;
    friend struct ContextExplorer;
    HookList & getHookList();
public:

    /// @deprecated
    HttpRequest* createRequest(const Uri & uri, DavixError** err);
    /// @deprecated
    HttpRequest* createRequest(const std::string & url, DavixError** err);
    /// @deprecated
    DavPosix* createDavPosix();

private:

};


/// runtime version string of the current davix library
/// @return version of davix
DAVIX_EXPORT const std::string & version();

/// runtime version string of the current HTTP backend library
/// @return version of backend library
DAVIX_EXPORT const std::string backendRuntimeVersion();

/// backend version _at the time of compilation_ which could
/// be different than runtime one
/// @return header version of backend library
DAVIX_EXPORT const std::string backendHeadersVersion();

}

#endif // DAVIXCONTEXT_HPP
