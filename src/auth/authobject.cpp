#include "authobject.hpp"

namespace Davix {

AuthObject::AuthObject() :
    d_ptr(NULL)
{
}

AuthObject::AuthObject(const AuthObject &auth){

}

AuthObject & AuthObject::operator=(const AuthObject &){

}

AuthObject::~AuthObject(){

}

//////////////////////////////////
//////////////////////////////////
/////////// X509 authentication
//////////////////////////////////

static std::string x509object_id("X509 Authentification");

struct X509AuthObjectInternal{
    X509AuthObjectInternal(const X509Credential &cred) :
        _cred(cred){ }
    X509AuthObjectInternal(const X509AuthObjectInternal & orig) :
        _cred(orig._cred){}

    X509Credential _cred;
};

X509AuthObject::X509AuthObject(const X509Credential &cred) :
    d_ptr(new X509AuthObjectInternal(cred))
{

}

X509AuthObject::X509AuthObject(const X509AuthObject &x509_auth) :
    d_ptr(new X509AuthObjectInternal(*(x509_auth.d_ptr)))
{

}

X509AuthObject & X509AuthObject::operator=(const X509AuthObject& x509_auth){
    if( this == &x509_auth)
        return *this;
    d_ptr = new X509AuthObjectInternal(*(x509_auth.d_ptr));
    return *this;
}

X509AuthObject::~X509AuthObject(){
    delete d_ptr;
}


const std::string & X509AuthObject::description() const{
    return x509object_id;
}

} // namespace Davix
