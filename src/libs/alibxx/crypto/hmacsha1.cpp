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


#include <davix_internal_config.hpp>
#include "hmacsha1.hpp"


#ifdef HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/sha.h>

static std::string hmac_sha1_openssl(const std::string & key, const std::string & data){
    unsigned char buffer_res[41];
    unsigned int buffer_len= 40;
    HMAC(EVP_sha1(), (const unsigned char*)key.c_str(), key.size(), (const unsigned char*) data.c_str(), data.size(), buffer_res, &buffer_len);
    return std::string((char*) buffer_res, buffer_len);
}

#endif

std::string hmac_sha1(const std::string & key, const std::string & data){
#ifdef HAVE_OPENSSL
    return hmac_sha1_openssl(key, data);
#else
#error "No support for hmac calculation"
#endif
}




