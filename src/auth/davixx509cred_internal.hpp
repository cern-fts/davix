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
