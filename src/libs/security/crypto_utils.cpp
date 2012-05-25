#include "crypto_utils.h"
#include <glibmm.h>

#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/err.h>


#include <global_def.h>

#define UTIL_SECURITY_BUFFER_SIZE 2048

#define log_function_debug(X, ...) Davix::davix_log_debug(X, ##__VA_ARGS__)




static const std::string & check_openssl_error(const char * domain, int err_code, const char* msg){
    unsigned long myerr = ERR_get_error();
    char buffer[UTIL_SECURITY_BUFFER_SIZE]= {0};
    ERR_error_string_n(myerr, buffer, UTIL_SECURITY_BUFFER_SIZE);
    ERR_remove_state(0);
    throw Glib::Error(Glib::Quark(domain), err_code, std::string(msg).append(buffer));
}



/**
  Try to convert a PEM credential to a p12 credential from memory string
  @throw Glib::Error
*/
ssize_t convert_x509_to_p12(const char *privkey, const char *clicert, const char* password, char *p12cert, size_t p12_size)
{
        X509      *cert;
        PKCS12    *p12;
        EVP_PKEY  *cert_privkey;
        size_t       bytes = 0;

        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();

        /* Read the private key file */
        if ((cert_privkey = EVP_PKEY_new()) == NULL){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_INSTANCE, "Unable to instance cert_privkey" );
        }

        log_function_debug(" convert_x509_to_p12 -> try to read private Key....");
        BIO *priv_key_mem = BIO_new_mem_buf((char*)privkey, -1);
        if (! (cert_privkey = PEM_read_bio_PrivateKey(priv_key_mem, NULL, NULL, (char*)password))){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_PRIVATEKEY, "Unable to open Private Key ");
        }

        log_function_debug(" convert_x509_to_p12 -> try to read User CERT ....");
        BIO *cert_mem = BIO_new_mem_buf((char*)clicert, -1);
        if (! (cert = PEM_read_bio_X509(cert_mem, NULL, NULL, NULL))){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_CERT, "Unable to open CERT ");
        }

        if ((p12 = PKCS12_new()) == NULL){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_INSTANCE, "Unable to creat pkcs12 ");
        }
        log_function_debug(" Merge Private Key and CERT to p12 ....");
        p12 = PKCS12_create(NULL, NULL, cert_privkey, cert, NULL, 0, 0, 0, 0, 0);
        if ( p12 == NULL){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_PKCS12, "Unable to convert pem to pkcs12, wrong format ");
        }

        BIO *p12_mem = BIO_new(BIO_s_mem());
        if(( bytes = i2d_PKCS12_bio(p12_mem, p12)) <0 ){
            check_openssl_error("convert_x509_to_p12", UTILS_SECURITY_ERROR_INSTANCE, "Unable to convert p12 to membuff ");
        }
        if( bytes+1 > p12_size)
            check_openssl_error("convert_x509_to_p12", ENOMEM, "No such space in buffer for key storage ");

        ssize_t ret = BIO_read(p12_mem, p12cert, p12_size);
        if(ret < 0){
           std::stringstream s;
           s << "Bad Read to the OpenSSL Bio handle " << ret << " " << p12_size << std::endl;
           throw Glib::Error(Glib::Quark("convert_x509_to_p12"), EINVAL, s.str());
        }
        p12cert[ret]= '\0';


        PKCS12_free(p12);


        // NOT MEMORY SAFE §§§§§§ investigations Needed
        return ret;
}
