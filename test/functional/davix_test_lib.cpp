
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


using namespace Davix;


int mycred_auth_callback_x509(void* userdata, const SessionInfo & info, X509Credential * cred, DavixError** err){
    Davix::DavixError* tmp_err= NULL;

    std::string path((char*) userdata);
    int ret = cred->loadFromFileP12(path, "", &tmp_err);

    if(ret != 0){
        fprintf(stderr, " FATAL authentification Error : %s", tmp_err->getErrMsg().c_str());
        DavixError::propagateError(err, tmp_err);
    }
    return ret;
}




char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff){
    snprintf(buff, s_buff, "%s/%s_%d%ld%ld",uri_dir, prefix, (int)getpid() ,(long) time(NULL), (long) rand());
    return buff;
}

void configure_grid_env(char * cert_path, RequestParams&  p){

    char login_passwd[strlen(cert_path)+1];
    char* pstr;
    strcpy(login_passwd, cert_path);
    pstr = strchr(login_passwd, ':');
    if( pstr != NULL){
        *pstr= '\0';
        pstr++;
        p.setClientLoginPassword(std::string(login_passwd), std::string(pstr));
    }

    p.setSSLCAcheck(false);
    p.setClientCertCallbackX509(&mycred_auth_callback_x509, cert_path);

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
