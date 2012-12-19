
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

char * generate_random_string_content(size_t size){
    char * res = (char*) malloc(size * sizeof(char));
    size_t i =0;
    while(i < size){
        res[i]= (char) (((rand()%2)?65:97)+(rand()%26));
        i++;
    }
    return res;
}
