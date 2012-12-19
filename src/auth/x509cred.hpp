#ifndef DAVIX_X509CRED_HPP
#define DAVIX_X509CRED_HPP

#include <status/davixstatusrequest.hpp>


namespace Davix {

struct X509CredentialInternal;

class X509Credential
{
public:
    X509Credential();
    ~X509Credential();

    /// load a credential from a PEM file
    /// @param key_file : path to the p12 credential file
    /// @param
    int loadFromFileP12(const std::string & p12_cred, const std::string & password, DavixError** err);


    bool hasCert() const;

private:
    X509CredentialInternal * d_ptr;
};

} // namespace DAvix

#endif // DAVIX_X509CRED_HPP
