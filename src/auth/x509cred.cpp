#include "x509cred.hpp"

namespace Davix {

struct X509CredentialInternal{
    int a;
};

X509Credential::X509Credential() :
    d_ptr(new X509CredentialInternal())
{
}

X509Credential::~X509Credential(){
    delete d_ptr;
}

} // namespace DAvix
