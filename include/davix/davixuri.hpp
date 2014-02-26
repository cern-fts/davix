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


#pragma once
#ifndef DAVIX_DAVIXURI_HPP
#define DAVIX_DAVIXURI_HPP

#include <string>
#include <status/davixstatusrequest.hpp>


/**
  @file davixuri.hpp
  @author Devresse Adrien
  @brief URI utilities functions for davix
 */

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix {

struct UriPrivate;

/// @class Uri
/// @brief Uri parser
/// convenience class for uri parsing
///
class DAVIX_EXPORT Uri
{
public:
    /// Construct an empty invalid Uri
    Uri();
    /// construct a new Davix Uri from a string URL
    Uri(const std::string & uri_string);
    /// Copy constructor
    Uri(const Uri & uri);
    ///
    /// \brief assignment operator
    /// \param orig
    /// \return
    ///
    Uri & operator=(const Uri & orig);
    virtual ~Uri();

    /// get a string representation of the full uri
    /// @return return the path or an empty string if error
    const std::string & getString() const;

    /// get the port number
    /// @return return the prot number of 0 if unspecified
    int getPort() const;

    /// get the protocol scheme
    ///  @return return the protocol scheme or an empty string if error
    const std::string & getProtocol() const;

    /// get the host name
    /// @return return the hostname or an empty string if error
    const std::string & getHost() const;

    /// get the path part of the Uri
    /// @return return the path of the Uri or an empty string if error
    const std::string & getPath()const;

    /// gextract user information from the URI
    /// @return return the path of the Uri or an empty string if error
    const std::string & getUserInfo() const;

    /// get a concatenation of the path and the query argument of the URI
    /// @return return a path + query arguments concatenation or an empty string if error
    const std::string & getPathAndQuery() const;

    /// get the query argument part of the uri
    /// @return return the query path string or an empty string if not exist or if error
    const std::string & getQuery() const;

    /// Status of the Uri
    /// see StatusCode::Code
    /// @return StatusCode::OK if success or StatusCode::UriParsingError if error
    StatusCode::Code getStatus() const;

    ///
    /// \brief test if two URI are equals
    /// \param u1
    /// \return true if equal, else false
    ///
    bool equal(const Uri & u1) const;

    ///
    /// \brief compare oepration
    /// \param u2
    /// \return true if u2 == current uri
    ///
    bool operator==(const Uri & u2) const;


    ///
    /// \brief Escape string
    /// \param str URL to escape
    /// \return encoded string
    ///
    static std::string escapeString(const std::string & str);

    ///
    /// \brief Unescape urI
    /// \param str URL to escape
    /// \return unencoded string
    ///
    static std::string unescapeString(const std::string & str);

    ///
    /// \brief create a new Uri from URI and a relative associated path
    /// \param uri original URI
    /// \param relPath relative path
    /// \return new URI from this path
    ///
    static Uri fromRelativePath(const Uri & uri, const std::string & relPath);


private:
    UriPrivate* d_ptr;
    void _init();
};


///
/// check the validity of a Davix::Uri
/// @param uri : davix uri
/// @param err : Davix Error report object
/// @return true if the uri is valid, or false and setup err with a string expression
bool DAVIX_EXPORT uriCheckError(const Uri & uri, DavixError ** err);


/// return associated std port for this request
/// return default http port or default SSL port if not precised
unsigned int DAVIX_EXPORT httpUriGetPort(const Uri & uri);

} // namespace Davix

std::ostream& operator<< (std::ostream& stream, const Davix::Uri & _u);

#endif // DAVIX_DAVIXURI_HPP
