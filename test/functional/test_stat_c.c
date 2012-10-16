#define _GNU_SOURCE

#include "test_stat_c.h"

#include <stdio.h>
#include <davix.h>
#include <glib.h>
#include <string.h>


int mycred_auth_callback(davix_auth_t token, const davix_auth_info_t* t, void* userdata, GError** err){
    GError * tmp_err=NULL;
    char login[2048];
    char passwd[2048];
    char *p,*auth_string =(char*) userdata;
    int ret ;
    gboolean login_password_auth_type = FALSE;
    memset(login,'\0', sizeof(char)*2048);

    if( (p = strchr(auth_string,':')) != NULL)
        login_password_auth_type = TRUE;

    if(login_password_auth_type ){
        *((char*) mempcpy(login, auth_string, p-auth_string)) = '\0';
        strcpy(passwd, p+1 );
        ret = davix_set_login_passwd_auth(token, login, passwd, &tmp_err);

    }else{
        ret = davix_set_pkcs12_auth(token, (const char*)userdata, (const char*)NULL, &tmp_err);
    }

    if(ret != 0){
        fprintf(stderr, " FATAL authentification Error : %s", tmp_err->message);
        g_propagate_error(err, tmp_err);
    }
    return ret;
}


int main(int argc, char** argv){
    if( argc < 2){
        printf(" Usage %s [url] [credential_path_p12]", argv[0]);
        return 0;
    }


    GError * tmp_err=NULL;
    int res =-1;
    struct stat st;
    davix_params_t p = NULL;

    davix_sess_t ctxt = davix_context_new(&tmp_err);

    if(!tmp_err && argc >=3){
        p = davix_params_new();
        davix_params_set_auth_callback(p, mycred_auth_callback, argv[2], &tmp_err);
        davix_params_set_ssl_check(p, FALSE, &tmp_err);
      //  davix_set_default_session_params(ctxt, p, NULL);

    }


    if(!tmp_err)
        res = davix_posix_stat(ctxt, p, argv[1], &st, &tmp_err);


    if(res == 0){
       printf("stat success \n" );
       printf(" atime : %ld \n",st.st_atime);
       printf(" mtime : %ld \n",st.st_mtime);
       printf(" ctime : %ld \n", st.st_ctime );
       printf(" mode : %d \n", st.st_mode );
       printf(" len : %ld \n", st.st_size );
    }else{
        printf(" error N°%d : %s \n", tmp_err->code, tmp_err->message);
    }
    davix_context_free(ctxt);
    davix_params_free(p);
    return res;
}

