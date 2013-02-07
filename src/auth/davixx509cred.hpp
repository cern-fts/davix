#ifndef DAVIX_X509CRED_HPP
#define DAVIX_X509CRED_HPP

#include <status/davixstatusrequest.hpp>


#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp for the C++ API or davix.h for the C API should be included."
#endif



/// @file davixx509cred.hpp
/// @brief utilities for X509 credential


namespace Davix {

struct X509CredentialInternal;
struct X509CredentialExtra;

/// @class X509Credential
/// @brief container for X509 certificate
///
class X509Credential
{
public:
    X509Credential();
    X509Credential(const X509Credential & orig);
    X509Credential & operator=(const X509Credential & orig);
    ~X509Credential();

    /// load a credential from a PKCS12 file
    /// @param filepath_p12_cred : path to the p12 credential file
    /// @param password : pass to decrypt the credential, empty string if nothing
    /// @param err : davix error report
    int loadFromFileP12(const std::string & filepath_p12_cred, const std::string & password, DavixError** err);

    /// load a credential from a PEM file
    /// support RFC-3820 proxy certificate, "globus" proxy  certificate and "VOMS" proxy certificate
    /// support for concatenated format
    ///
    /// to use a concatenated PEM cred, just set filepath_priv_key = filepath_cred = concat cred path
    ///
    /// @param filepath_priv_key : path to the private key file
    /// @param filepath_cred : path to the credential file
    /// @param password : pass to decrypt the credential, empty string if unencrypted
    /// @param err : davix error report
    int loadFromFilePEM(const std::string & filepath_priv_key, const std::string & filepath_cred,
                                const std::string & password, DavixError** err);


    /// check if the container owns a certificate
    /// @return true if contains a valid certificate, false if empty
    bool hasCert() const;

private:
    X509CredentialInternal * d_ptr;

    friend struct X509CredentialExtra;
};

} // namespace DAvix

#endif // DAVIX_X509CRED_HPP
