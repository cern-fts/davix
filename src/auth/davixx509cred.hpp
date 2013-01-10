#ifndef DAVIX_X509CRED_HPP
#define DAVIX_X509CRED_HPP

#include <status/davixstatusrequest.hpp>


#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


/// @file davixx509cred.hpp
/// @brief utility tools for X509 credential manipulation


namespace Davix {

struct X509CredentialInternal;
struct X509CredentialExtra;

class X509Credential
{
public:
    X509Credential();
    X509Credential(const X509Credential & orig);
    X509Credential & operator=(const X509Credential & orig);
    ~X509Credential();

    /// load a credential from a PEM file
    /// @param key_file : path to the p12 credential file
    /// @param password : pass to decrypt the credential, empty string if nothing
    /// @param err : davix error report
    int loadFromFileP12(const std::string & filepath_p12_cred, const std::string & password, DavixError** err);


    /// check if contains a valid certificate
    /// @return true if contains a valid certificate, else false
    bool hasCert() const;

private:
    X509CredentialInternal * d_ptr;

    friend struct X509CredentialExtra;
};

} // namespace DAvix

#endif // DAVIX_X509CRED_HPP
