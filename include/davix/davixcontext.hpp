/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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

    /// clone this instance to a new context dynamically allocated,
    Context* clone();



    ///  enable or disablet the session caching
    void setSessionCaching(bool caching);

    bool getSessionCaching() const;


    /// @deprecated
    HttpRequest* createRequest(const Uri & uri, DavixError** err);
    /// @deprecated
    HttpRequest* createRequest(const std::string & url, DavixError** err);
    /// @deprecated
    DavPosix* createDavPosix();

private:
    // internal context
    ContextInternal* _intern;

    friend class DavPosix;
    friend struct ContextExplorer;
};


/// version string of the current davix library
/// @return version of davix
DAVIX_EXPORT const std::string & version();

}

#endif // DAVIXCONTEXT_HPP
