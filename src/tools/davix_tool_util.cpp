#include "davix_tool_util.hpp"

#include <string>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <simple_getpass/simple_get_pass.h>

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
    opts.params.setClientLoginPasswordCallback(&DavixToolsAuthCallbackLoginPassword, &opts);
    return 0;
}

FILE* configure_fstream(const Tool::OptParams & opts, const std::string & scope, DavixError** err){
    if(opts.output_file_path.empty() == false){
        FILE* f = fopen(opts.output_file_path.c_str(),"w");
        if(f == NULL){
            davix_errno_to_davix_error(errno, scope, std::string(" ").append(opts.output_file_path), err);
            return NULL;
        }

        return f;

    }
    return stdout;
}



void err_display(DavixError ** err){
    if(err && *err){
        std::cerr << "("<< (*err)->getErrScope() <<") Error: "<< (*err)->getErrMsg() << std::endl;
        DavixError::clearError(err);
    }
}

std::string mode_to_stringmode(mode_t mode){
    mode_t tmp_mode = mode;
    //static const char * strv= "xwrxwrxwr";
    char res[11];
    memset(res,'-', sizeof(10));
    res[10]='\0';
    for(int i=0; i <9; i++){
        res[9-i] = ( mode = (tmp_mode >> 1)) & 0x01;
    }
    res[0]= S_ISDIR(mode);
    return std::string(res);
}

int DavixToolsAuthCallbackLoginPassword(void* userdata, const SessionInfo & info, std::string & login, std::string & password,
                                        int count, DavixError** err){
    OptParams* opts = (OptParams*) userdata;
    int ret = -1;
    if(opts->userlogpasswd.first.empty() == false){
        login = opts->userlogpasswd.first;
        password = opts->userlogpasswd.second;
        ret =0;
    }else{
        char l[1024];
        char p[1024];

        if(count > 0)
            std::cout << "Authentication Failure, try again:\n";
        else
            std::cout << "Authentication needed:\n";
        std::cout << "Login: ";
        std::cout.flush();
        std::cin.getline(l, 1024);
        if( strlen(l) > 0){
            std::cout << "Password: ";
            std::cout.flush();
            if(simple_get_pass(p, 1024) > 0){
                login = std::string(l);
                password = std::string(p);
                decimate_passwd(p);
                decimate_passwd(l);
                ret =0;
            }

        }
    }
    std::cout << std::endl;
    if(ret < 0)
        DavixError::setupError(err, "Davix::Tool::Auth",
                               StatusCode::LoginPasswordError,
                               "No valid login/password provided");
    return ret;
}

}
}
