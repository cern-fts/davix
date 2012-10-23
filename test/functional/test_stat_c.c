#define _GNU_SOURCE

#include "test_stat_c.h"

#include <stdio.h>
#include <davix.h>
#include <glib.h>
#include <string.h>
#include "davix_test_lib.h"



int main(int argc, char** argv){
    if( argc < 2){
        printf(" Usage %s [url] [credential_path_p12]", argv[0]);
        return 0;
    }


    davix_error_t tmp_err=NULL;
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
        printf(" error N°%d : %s \n", davix_error_code(tmp_err), davix_error_msg(tmp_err));
    }
    davix_context_free(ctxt);
    davix_params_free(p);
    return res;
}

