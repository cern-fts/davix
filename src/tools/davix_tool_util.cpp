#include "davix_tool_util.hpp"



namespace Davix{

namespace Tool{


int setup_credential(OptParams & opts, DavixError** err){
    // setup client side credential
    if(opts.cred_path.empty() == false){
        X509Credential cred;
        if( cred.loadFromFilePEM(opts.cred_path, opts.cred_path, "", err) <0){
            return -1;
        }
        opts.params.setClientCertX509(cred);
    }

    // setup client login / password
    if( !opts.userlogpasswd.first.empty()){
      opts.params.setClientLoginPassword(opts.userlogpasswd.first,
                                         opts.userlogpasswd.second);
    }
    return 0;
}


void err_display(DavixError ** err){
    if(err && *err){
        std::cerr << "Error: "<< (*err)->getErrMsg() << std::endl;
        DavixError::clearError(err);
    }
}

std::string mode_to_stringmode(mode_t mode){
    mode_t tmp_mode = mode;
    const char * strv= "xwrxwrxwr";
    char res[11];
    memset(res,'-', sizeof(10));
    res[10]='\0';
    for(int i=0; i <9; i++){
        res[9-i] = ( mode = (tmp_mode >> 1)) & 0x01;
    }
    res[0]= S_ISDIR(mode);
    return std::string(res);
}

}
}
