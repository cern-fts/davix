#include <config.h>
#include <ne_ssl.h>

#include "davixx509cred_internal.hpp"



namespace Davix {

struct X509CredentialInternal{
    X509CredentialInternal() :
        _cred(NULL)
    {}

    X509CredentialInternal(const X509CredentialInternal & orig) :
        _cred((orig._cred)?ne_ssl_dup_client_cert(orig._cred):NULL)
    {}

    X509CredentialInternal & operator=(const X509CredentialInternal & orig){
        _cred= (orig._cred)?ne_ssl_dup_client_cert(orig._cred):NULL;
        return *this;
    }

    ~X509CredentialInternal(){
        clear_cert();
    }

    void clear_cert(){
        if(_cred){
            ne_ssl_clicert_free(_cred);
            _cred = NULL;
        }
    }

    ne_ssl_client_cert * _cred;
};

X509Credential::X509Credential() :
    d_ptr(new X509CredentialInternal())
{
}

X509Credential::X509Credential(const X509Credential &orig) :
    d_ptr(new X509CredentialInternal(*(orig.d_ptr)))
{

}

X509Credential & X509Credential::operator =( const X509Credential & orig){
    if(this == & orig)
        return *this;
    delete d_ptr;
    d_ptr = new X509CredentialInternal(*(orig.d_ptr));
    return *this;
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


int X509Credential::loadFromFilePEM(const std::string & filepath_priv_key, const std::string & filepath_cred,
                                    const std::string & password, DavixError** err){
    d_ptr->clear_cert();
    if( (d_ptr->_cred = ne_ssl_clicert_pem_read(filepath_priv_key.c_str(), filepath_cred.c_str(),
                                                (password.size() == 0)?NULL:password.c_str())) == NULL){
        Davix::DavixError::setupError(err, davix_scope_x509cred(), StatusCode::CredentialNotFound, std::string("Impossible to load PEM credential : ") + filepath_priv_key + " " + filepath_cred);
        d_ptr->clear_cert();
        return -1;
    }
    return 0;
}

bool X509Credential::hasCert() const{
    return (d_ptr->_cred != NULL);
}

ne_ssl_client_cert* X509CredentialExtra::extract_ne_ssl_clicert(const X509Credential &cred)
{
        return (cred.d_ptr->_cred);
}

} // namespace DAvix


DAVIX_C_DECL_BEGIN

davix_x509_cert_t davix_x509_cert_new(){
    return (davix_x509_cert_t) new Davix::X509Credential();
}

/// return true if certificate container contain a valid credential, else false
bool davix_x509_cert_has_cert(davix_x509_cert_t cred){
    g_assert(cred);
    return ((Davix::X509Credential*) cred)->hasCert();
}

/// load a pkcs12 certificate
int davix_x509_cert_load_from_p12(davix_x509_cert_t cred, const char * path, const char* passwd, davix_error_t* err){
    g_assert(cred && path);
    return ((Davix::X509Credential*) cred)->loadFromFileP12(path, ((passwd)?(passwd):""), (Davix::DavixError**)err);
}

/// free a container for X509 certificate
void davix_x509_cert_free(davix_x509_cert_t cred){
    if(cred)
        delete (Davix::X509Credential*) cred;
}


DAVIX_C_DECL_END
