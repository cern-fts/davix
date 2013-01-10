#ifndef DAVIX_X509CRED_H
#define DAVIX_X509CRED_H

#include <davix_types.h>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


/// @file davixx509cred.hpp
/// @brief utility tools for X509 credential manipulation, C Bindings


DAVIX_C_DECL_BEGIN

typedef struct davix_x509_certificate_s* davix_x509_cert_t;



/// create a new container for X509 certificate
davix_x509_cert_t davix_x509_cert_new();

/// return true if certificate container contain a valid credential, else false
bool davix_x509_cert_has_cert(davix_x509_cert_t cred);

/// load a pkcs12 certificate
int davix_x509_cert_load_from_p12(davix_x509_cert_t cred, const char * path, const char* passwd, davix_error_t* err);

/// free a container for X509 certificate
void davix_x509_cert_free(davix_x509_cert_t cred);


DAVIX_C_DECL_END

#endif // DAVIX_X509CRED_HPP
