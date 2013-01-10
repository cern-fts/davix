#ifndef DAVIX_AUTH_H
#define DAVIX_AUTH_H


#include <auth/davixx509cred.h>


#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

/// @file davixauth.h
/// @brief Authentication support for davix, C Bindings
/// support for client cert x509, login password


DAVIX_C_DECL_BEGIN

typedef struct davix_session_info_s* davix_session_info_t;

///
/// callback for advanced authentification with client cert X509
/// @param userdata : user defined data
/// @param info : Session info, contains information about server requesting the certificate
/// @param cert : Client side credential to provide
/// @param err : error object to set if an error occures
/// @return MUST return 0 if credential if provided with success or != 0 if error occures
typedef int (*davix_auth_cb_client_cert_x509)(void* userdata, const davix_session_info_t info, davix_x509_certificate_t cert, davix_error_t* err);


///
/// callback for advanced authentification with client cert X509
/// @param userdata : user defined data
/// @param info : Session info, contains information about server requesting the certificate
/// @param login : login to use
/// @param password : password to use
/// @param count : number of try
/// @return MUST return 0 if success, or !=0 if an error has occures
typedef int (*davix_auth_cb_login_passwd)(void* userdata, const davix_session_info_t info, const char* login, const char* password,
                                        int count, davix_error_t* err);


DAVIX_C_DECL_END

#endif // DAVIX_AUTHOBJECT_H
