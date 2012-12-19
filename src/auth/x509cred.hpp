#ifndef DAVIX_X509CRED_HPP
#define DAVIX_X509CRED_HPP




namespace Davix {

struct X509CredentialInternal;

class X509Credential
{
public:
    X509Credential();
    ~X509Credential();

private:
    X509CredentialInternal * d_ptr;
};

} // namespace DAvix

#endif // DAVIX_X509CRED_HPP
