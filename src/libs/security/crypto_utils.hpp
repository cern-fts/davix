#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <cstring>
#include <iostream>

ssize_t convert_x509_to_p12(const char *privkey, const char *clicert, const char* password, char *p12cert, size_t p12_size);

#define UTILS_SECURITY_ERROR_INSTANCE 100
#define UTILS_SECURITY_ERROR_PRIVATEKEY 101
#define UTILS_SECURITY_ERROR_CERT 102
#define UTILS_SECURITY_ERROR_PKCS12 103
#define UTILS_SECURITY_ERROR_ENOMEM 104

#endif // CRYPTO_UTILS_H
