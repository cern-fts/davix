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

#ifndef DAVIX_X509CRED_HPP
#define DAVIX_X509CRED_HPP

#include <status/davixstatusrequest.hpp>


#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp for the C++ API or davix.h for the C API should be included."
#endif



/// @file davixx509cred.hpp
/// @brief utilities for X509 credential


namespace Davix {

struct X509CredentialInternal;
struct X509CredentialExtra;

/// @class X509Credential
/// @brief X509 certificate
///
class DAVIX_EXPORT X509Credential
{
public:
    ///
    /// \brief default constructor
    ///
    X509Credential();
    ///
    /// \brief copy constructor
    ///
    X509Credential(const X509Credential & orig);
    ///
    /// \brief assignment operator
    ///
    X509Credential & operator=(const X509Credential & orig);
    /// \brief destructor
    ~X509Credential();

    /// load a credential from a PKCS12 file
    /// @param filepath_p12_cred : path to the p12 credential file
    /// @param password : pass to decrypt the credential, empty string if nothing
    /// @param err : davix error report
    int loadFromFileP12(const std::string & filepath_p12_cred, const std::string & password, DavixError** err);

    /// load a credential from a PEM file
    /// support RFC-3820 proxy certificate, "globus" proxy  certificate and "VOMS" proxy certificate
    /// support for concatenated format
    ///
    /// to use a concatenated PEM cred, just set filepath_priv_key = filepath_cred = concat cred path
    ///
    /// @param filepath_priv_key : path to the private key file
    /// @param filepath_cred : path to the credential file
    /// @param password : pass to decrypt the credential, empty string if unencrypted
    /// @param err : davix error report
    int loadFromFilePEM(const std::string & filepath_priv_key, const std::string & filepath_cred,
                                const std::string & password, DavixError** err);


    /// check if the object contain a credential
    /// @return true if contains a valid certificate, false if empty
    bool hasCert() const;

private:
    X509CredentialInternal * d_ptr;

    friend struct X509CredentialExtra;
};


} // namespace Davix

#endif // DAVIX_X509CRED_HPP
