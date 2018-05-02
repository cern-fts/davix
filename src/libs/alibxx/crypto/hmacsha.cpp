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


#include <iostream>
#include <davix_internal_config.hpp>
#include "hmacsha.hpp"


#ifdef HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

// 0 for sha1, 1 for sha256
#define HMAC_SHA1 0
#define HMAC_SHA256 1

static std::string hmac_openssl(int type, const std::string & key, const std::string & data) {
    unsigned char buffer_res[65];
    unsigned int buffer_len = (type == HMAC_SHA256) ? 64 : 40;
    const EVP_MD *hashtype = (type == HMAC_SHA256) ? EVP_sha256() : EVP_sha1();
    HMAC(hashtype, (const unsigned char*)key.c_str(), key.size(),
            (const unsigned char*) data.c_str(), data.size(), buffer_res, &buffer_len);
    return std::string((char*) buffer_res, buffer_len);
}

#endif

std::string hmac_sha1(const std::string & key, const std::string & data){
#ifdef HAVE_OPENSSL
    return hmac_openssl(HMAC_SHA1, key, data);
#else
#error "No support for hmac calculation"
#endif
}

std::string hmac_sha256(const std::string & key, const std::string & data){
#ifdef HAVE_OPENSSL
    return hmac_openssl(HMAC_SHA256, key, data);
#else
#error "No support for hmac calculation"
#endif
}

std::string sha256(const std::string input) {
#ifdef HAVE_OPENSSL
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    return std::string((char*) hash, SHA256_DIGEST_LENGTH);
#else
#error "No support for sha256 calculation"
#endif
}

std::string rsasha256(const std::string &key, const std::string &data) {
#ifdef HAVE_OPENSSL

    BIO* bio = BIO_new_mem_buf( (void*) key.data(), key.size());
    if(!bio) return "";

    EVP_PKEY* private_key = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    if(!private_key) {
        BIO_free(bio);
        return "";
    }

    RSA *rsa = EVP_PKEY_get1_RSA(private_key);
    if(!rsa) {
      EVP_PKEY_free(private_key);
      BIO_free(bio);
      return "";
    }

    std::string digest = sha256(data);

    unsigned int siglen;
    char retbuff[2048];
    if(RSA_sign(NID_sha256, (const unsigned char*) digest.c_str(), digest.size(), (unsigned char*) retbuff, &siglen, rsa) != 1) {
        siglen = 0;
    }

    RSA_free(rsa);
    EVP_PKEY_free(private_key);
    BIO_free(bio);

    return std::string(retbuff, siglen);
#else
#error "No support for sha256 calculation"
#endif
}
