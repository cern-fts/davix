
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

int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, davix_error_t* err){
    davix_error_t tmp_err=NULL;
    char login[2048];
    char passwd[2048];
    char *p,*auth_string =(char*) userdata;
    int ret ;
    gboolean login_password_auth_type = FALSE;
    memset(login,'\0', sizeof(char)*2048);

    if( (p = strchr(auth_string,':')) != NULL)
        login_password_auth_type = TRUE;

    if(login_password_auth_type ){
        *((char*)mempcpy(login, auth_string, p-auth_string)) ='\0';
        strcpy(passwd, p+1 );
        ret = davix_set_login_passwd_auth(token, login, passwd, &tmp_err);

    }else{
        ret = davix_set_pkcs12_auth(token, (const char*)userdata, (const char*)NULL, &tmp_err);
    }

    if(ret != 0){
        fprintf(stderr, " FATAL authentification Error : %s", davix_error_msg(tmp_err));
        davix_error_propagate(err, tmp_err);
    }
    return ret;
}







char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff){
    snprintf(buff, s_buff, "%s/%s_%d%ld%ld",uri_dir, prefix, (int)getpid() ,(long) time(NULL), (long) rand());
    return buff;
}

char * generate_random_string_content(size_t size){
    char * res = malloc(size * sizeof(char));
    size_t i =0;
    while(i < size){
        res[i]= (char) (((rand()%2)?65:97)+(rand()%26));
        i++;
    }
    return res;
}
