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

#ifndef DAVIX_X509CRED_INTERNAL_HPP
#define DAVIX_X509CRED_INTERNAL_HPP

#include <auth/davixx509cred.hpp>
#include <ne_ssl.h>

namespace Davix{


// SSL Ops
ne_ssl_client_cert *SSL_X509_Pem_Read(const std::string & pkeyfile, const std::string & credfile,
                                      const std::string & password, DavixError** err);


struct X509CredentialExtra{

    static ne_ssl_client_cert* extract_ne_ssl_clicert(const X509Credential & cred);

    static bool get_x509_info(const X509Credential &cred,
            std::string* ucert, std::string* ukey, std::string* passwd);

};

}





#endif // DAVIX_X509CRED_INTERNAL_HPP
