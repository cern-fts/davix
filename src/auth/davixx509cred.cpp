#include <config.h>
#include <ne_ssl.h>
#include <cassert>

#include "davixx509cred_internal.hpp"



namespace Davix {

struct X509CredentialInternal{
    X509CredentialInternal() :
        _cred(NULL),
        x509_ucert(),
        x509_ukey(),
        x509_passwd(),
        pemLoaded(false)
    {}

    X509CredentialInternal(const X509CredentialInternal & orig) :
        _cred((orig._cred)?ne_ssl_dup_client_cert(orig._cred):NULL),
        x509_ucert(orig.x509_ucert), x509_ukey(orig.x509_ukey),
        x509_passwd(orig.x509_passwd), pemLoaded(orig.pemLoaded)
    {}

    X509CredentialInternal & operator=(const X509CredentialInternal & orig){
        _cred= (orig._cred)?ne_ssl_dup_client_cert(orig._cred):NULL;

        x509_ucert.assign(orig.x509_ucert);
        x509_ukey.assign(orig.x509_ukey);
        x509_passwd.assign(orig.x509_passwd);
        pemLoaded = orig.pemLoaded;

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
        pemLoaded = false;
        x509_ucert.clear();
        x509_ukey.clear();
        x509_passwd.clear();
    }

    ne_ssl_client_cert * _cred;

    // Remember pem location
    std::string x509_ucert;
    std::string x509_ukey;
    std::string x509_passwd;
    bool        pemLoaded;
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
    if( (d_ptr->_cred = SSL_X509_Pem_Read(filepath_priv_key, filepath_cred, password, err)) != NULL){
        d_ptr->x509_ucert  = filepath_cred;
        d_ptr->x509_ukey   = filepath_priv_key;
        d_ptr->x509_passwd = password;
        d_ptr->pemLoaded   = true;
        return 0;
    }
    return -1;
}

bool X509Credential::hasCert() const{
    return (d_ptr->_cred != NULL);
}

ne_ssl_client_cert* X509CredentialExtra::extract_ne_ssl_clicert(const X509Credential &cred)
{
        return (cred.d_ptr->_cred);
}

bool X509CredentialExtra::get_x509_info(const X509Credential &cred,
        std::string* ucert, std::string* ukey, std::string* passwd)
{
    ucert->assign(cred.d_ptr->x509_ucert);
    ukey->assign(cred.d_ptr->x509_ukey);
    passwd->assign(cred.d_ptr->x509_passwd);
    return cred.d_ptr->pemLoaded;
}

} // namespace DAvix



