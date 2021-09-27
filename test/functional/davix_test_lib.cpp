
#include "davix_test_lib.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sstream>


using namespace Davix;


int mycred_auth_callback_x509(void* userdata, const SessionInfo & info, X509Credential * cred, DavixError** err){
    (void) info;
    Davix::DavixError* tmp_err= NULL;

    std::string path((char*) userdata);
    int ret = cred->loadFromFileP12( path, "", &tmp_err);

    if(ret != 0){
        fprintf(stderr, " FATAL authentication Error : %s", tmp_err->getErrMsg().c_str());
        DavixError::propagateError(err, tmp_err);
    }
    return ret;
}




char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff){
    snprintf(buff, s_buff, "%s/%s_%d%ld%ld",uri_dir, prefix, (int)getpid() ,(long) time(NULL), (long) rand());
    return buff;
}

Davix::Uri generate_random_uri(const Davix::Uri & url_directory, const std::string & prefix){
    std::ostringstream ss;
    ss << url_directory << "/" << prefix << static_cast<int>(getpid()) << static_cast<long>(time(NULL)) << static_cast<long>(rand());
    return Davix::Uri(ss.str());
}

void configure_grid_env(char * cert_path, RequestParams&  p){

    if(strcmp(cert_path, "proxy") == 0){ // VOMS PROXY MODE
        X509Credential x;
        std::string proxy_path;
        char* p_str;
        if ( (p_str = getenv("X509_USER_PROXY") ) != NULL ){
            proxy_path = p_str;
        }else{
            std::ostringstream ss;
            ss << "/tmp/x509up_u" << getuid();
            proxy_path = ss.str();
        }
        x.loadFromFilePEM(proxy_path,proxy_path,"", NULL);
        p.setClientCertX509(x);
    } else{
        char login_passwd[strlen(cert_path)+1];
        char* pstr;
        strcpy(login_passwd, cert_path);
        pstr = strchr(login_passwd, ':');
        if( pstr != NULL){
            *pstr= '\0';
            pstr++;
            p.setClientLoginPassword(std::string(login_passwd), std::string(pstr));
        }
        p.setClientCertCallbackX509(&mycred_auth_callback_x509, cert_path);
    }

    // add standard grid certificate for wlcg test
    p.addCertificateAuthorityPath("/etc/grid-security/certificates/");
    // add wrong cert path for testing purpose
    p.addCertificateAuthorityPath("/Iamnotexisting");
    p.addCertificateAuthorityPath("");
    p.addCertificateAuthorityPath("/etc/group"); // add a file



}


char * generate_random_string_content(size_t size){
    char * res = (char*) malloc(size * sizeof(char));
    size_t i =0;
    while(i < size){
        res[i]= (char) (((rand()%2)?65:97)+(rand()%26));
        i++;
    }
    return res;
}


/*
DAVIX_C_DECL_BEGIN


void configure_grid_env_c(char * cert_path, davix_params_t  params){


    davix_error_t tmp_err=NULL;
    char login_passwd[strlen(cert_path)+1];
    char* pstr;

    strcpy(login_passwd, cert_path);
    pstr = strchr(login_passwd, ':');
    if( pstr != NULL){
        *pstr= '\0';
        pstr++;
        davix_params_set_login_passwd(params, login_passwd, pstr);
    }else{
        davix_x509_cert_t cred = davix_x509_cert_new();
        davix_x509_cert_load_from_p12(cred, cert_path, "", &tmp_err);
        if(tmp_err){
            std::cerr << " failure when load cert : " << davix_error_msg(tmp_err) << std::endl;
            exit(-1);
        }
        davix_params_set_client_cert_X509(params, cred);
        davix_x509_cert_free(cred);

    }

    davix_params_set_ssl_check(params, false, &tmp_err);
}

DAVIX_C_DECL_END
*/
