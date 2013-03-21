#ifndef DAVIX_X509CRED_INTERNAL_HPP
#define DAVIX_X509CRED_INTERNAL_HPP

#include <auth/davixx509cred.hpp>
#include <ne_ssl.h>

namespace Davix{




struct X509CredentialExtra{

    static ne_ssl_client_cert* extract_ne_ssl_clicert(const X509Credential & cred);

};

}





#endif // DAVIX_X509CRED_INTERNAL_HPP
