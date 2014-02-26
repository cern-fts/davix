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

#ifndef DAVIX_AUTH_HPP
#define DAVIX_AUTH_HPP


#include <string>

#include <auth/davixx509cred.hpp>


#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp for the C++ API or davix.h for the C API should be included."
#endif

/// @file davixauth.hpp
/// @brief Authentication support for davix
/// support for client cert x509, login password, S3 tokens


namespace Davix {

///  @class SessionInfo
///  @brief server related info
class DAVIX_EXPORT SessionInfo{
public:
    /// \brief tada
    void* a;
/// TODO: fill with server side infos

};

///
/// callback for advanced authentification with client cert X509
/// @param userdata : user defined data
/// @param info : Session info, contains information about server requesting the certificate
/// @param cert : Client side credential to provide
/// @param err : error object to set if an error occures
/// @return MUST return 0 if credential if provided with success or != 0 if error occures
typedef int (*authCallbackClientCertX509)(void* userdata, const SessionInfo & info, X509Credential * cert, DavixError** err);


///
/// callback for advanced authentification with client cert X509
/// @param userdata : user defined data
/// @param info : Session info, contains information about server requesting the certificate
/// @param login : login to use
/// @param password : password to use
/// @param count : number of try
/// @return MUST return 0 if success, or !=0 if an error has occures
typedef int (*authCallbackLoginPasswordBasic)(void* userdata, const SessionInfo & info, std::string & login, std::string & password,
                                        int count, DavixError** err);


/// \cond PRIVATE_SYMBOLS
std::string getAwsAuthorizationField(const std::string & stringToSign, const std::string & private_key, const std::string & access_key);
/// \endcond PRIVATE_SYMBOLS

} // namespace Davix

#endif // DAVIX_AUTHOBJECT_HPP
