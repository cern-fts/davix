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
        strncpy(login, auth_string, p-auth_string);
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

    davix_sess_t ctxt = davix_session_new(&tmp_err);

    if(!tmp_err && argc >=3){
        davix_set_auth_callback(ctxt, mycred_auth_callback, argv[2], &tmp_err);
        davix_set_ssl_check(ctxt, FALSE, &tmp_err);
    }


    if(!tmp_err)
        res = davix_stat(ctxt, argv[1], &st, &tmp_err);


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
    davix_session_free(ctxt);
    return res;
}

