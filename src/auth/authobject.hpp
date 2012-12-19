#ifndef DAVIX_AUTHOBJECT_HPP
#define DAVIX_AUTHOBJECT_HPP


#include <string>

#include <davix_types.h>
#include <auth/x509cred.hpp>

namespace Davix {

struct AuthObjectInternal;
struct X509AuthObjectInternal;
class X509Credential;

///
/// @class AuthObject
/// Generic authentification object
/// Derivative of this class implement all the authentification schema : Login + password, X509 credential, Kerberos, etc...
///
class AuthObject
{
public:
    AuthObject();
    AuthObject(const AuthObject & auth);
    AuthObject & operator=(const AuthObject & auth);
    virtual ~AuthObject();


    virtual const std::string & description() const =0;

    AuthObjectInternal* d_ptr;
};

///
/// @class X509AuthObject
/// provide authentication with X509 client side credential
/// Support all kind of credential supported by Davix::X509Credential
///
class X509AuthObject : public AuthObject{
public:
    X509AuthObject(const X509Credential & cred);
    X509AuthObject(const X509AuthObject & x509_auth);
    X509AuthObject & operator=(const X509AuthObject & x509_auth);
    virtual ~X509AuthObject();

    virtual const std::string & description() const;

private:
    X509AuthObjectInternal* d_ptr;
};



} // namespace Davix

#endif // DAVIX_AUTHOBJECT_HPP
