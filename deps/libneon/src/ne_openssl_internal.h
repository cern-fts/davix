#ifndef DAVIX_OPENSSL_INTERNAL_H
#define DAVIX_OPENSSL_INTERNAL_H

/// internal openssl declaration



#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/opensslv.h>

#include <libneon/src/ne_ssl.h>
#include <libneon/src/ne_privssl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ne_ssl_dname_s {
    X509_NAME *dn;
};


struct ne_ssl_certificate_s {
    ne_ssl_dname subj_dn, issuer_dn;
    X509 *subject;
    STACK_OF(X509) *chain;
    ne_ssl_certificate *issuer;
    char *identity;
};

struct ne_ssl_client_cert_s {
    PKCS12 *p12;
    int decrypted; /* non-zero if successfully decrypted. */
    ne_ssl_certificate cert;
    EVP_PKEY *pkey;
    char *friendly_name;
};


ne_ssl_certificate *populate_cert(ne_ssl_certificate *cert, X509 *x5);


#ifdef __cplusplus
}
#endif

#endif // DAVIX_OPENSSL_INTERNAL_H
