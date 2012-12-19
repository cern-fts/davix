#include "x509cred.hpp"

#include <ne_ssl.h>

namespace Davix {

struct X509CredentialInternal{
    X509CredentialInternal() :
        _cred(NULL)
    {}

    X509CredentialInternal(const X509CredentialInternal & orig) :
        _cred(ne_ssl_dup_client_cert(orig._cred))
    {}

    ~X509CredentialInternal(){
        clear_cert();
    }

    void clear_cert(){
        if(_cred)
            ne_ssl_clicert_free(_cred);
    }

    ne_ssl_client_cert * _cred;
};

X509Credential::X509Credential() :
    d_ptr(new X509CredentialInternal())
{
}

X509Credential::~X509Credential(){
    delete d_ptr;
}


int X509Credential::loadFromFileP12(const std::string &p12_cred, const std::string & passwd, DavixError **err){
    d_ptr->clear_cert();
    if( (d_ptr->_cred = ne_ssl_clicert_read(p12_cred.c_str())) == NULL){
        Davix::DavixError::setupError(err, davix_scope_x509cred(),StatusCode::CredentialNotFound, std::string("Impossible to load credential ").append(p12_cred));
        return -1;
    }

    if( ne_ssl_clicert_encrypted(d_ptr->_cred) !=0
            && ne_ssl_clicert_decrypt(d_ptr->_cred, passwd.c_str()) !=0){
        Davix::DavixError::setupError(err, davix_scope_x509cred(), StatusCode::LoginPasswordError, std::string("Impossible to decrypt the credential  ").append(p12_cred).append(" with the provided password"));
        d_ptr->clear_cert();
        return -1;
    }
    return 0;
}

bool X509Credential::hasCert() const{
    return (d_ptr->_cred != NULL);
}

} // namespace DAvix
