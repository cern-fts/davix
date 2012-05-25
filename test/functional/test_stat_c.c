#include "test_stat_c.h"

#include <stdio.h>
#include <davix.h>


int main(int argc, char** argv){
    if( argc < 2){
        printf(" Usage %s [url]", argv[0]);
        return 0;
    }

    GError * tmp_err=NULL;
    int res =-1;
    struct stat st;

    davix_sess_t ctxt = davix_session_new(&tmp_err);
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

