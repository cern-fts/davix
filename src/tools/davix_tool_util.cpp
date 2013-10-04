#include "davix_tool_util.hpp"

#include <string>
#include <algorithm>
#include <fcntl.h>
#include <simple_getpass/simple_get_pass.h>

namespace Davix{

namespace Tool{


size_t ask_user_login(std::string & login){
    char l[1024] ={0};
    (std::cout << "Login: ").flush();
    std::cin.getline(l, 1023);
    login.assign(l);
    std::fill(l, l+1024,'\0');
    return login.size();
}


size_t ask_user_passwd(std::string & passwd){
    char p[1024] ={0};
    std::cout << "Password: ";
    std::cout.flush();
    if(simple_get_pass(p, 1023) > 0){
        passwd.assign(p);
        std::fill(p, p+1024,'\0');
        return passwd.size();
    }
    return 0;
}

int setup_credential(OptParams & opts, DavixError** err){
    // setup client side credential
    if(opts.cred_path.empty() == false){
        X509Credential cred;
        if( cred.loadFromFilePEM( ((opts.priv_key.empty()== false)?(opts.priv_key):(opts.cred_path)),
                                  opts.cred_path,
                                  "",
                                  err) <0){
            return -1;
        }
        opts.params.setClientCertX509(cred);
    }

    // setup client login / password
    opts.params.setClientLoginPasswordCallback(&DavixToolsAuthCallbackLoginPassword, &opts);

    //setup aws creds
    if(opts.aws_auth.first.empty() == false){
        opts.params.setAwsAuthorizationKeys(opts.aws_auth.first, opts.aws_auth.second);
        opts.params.setProtocol(RequestProtocol::AwsS3);
    }

    return 0;
}

int get_output_fstream(const Tool::OptParams & opts, const std::string & scope, DavixError** err){
    int fd = -1;

    if(opts.output_file_path.empty() == false){
        if((fd = open(opts.output_file_path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) <0  ){
            davix_errno_to_davix_error(errno, scope, std::string("for destination file ").append(opts.output_file_path), err);
            return -1;
        }
    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;
}

int get_input_fstream(const Tool::OptParams & opts, const std::string & scope, DavixError** err){
    int fd = -1;
    if(opts.input_file_path.empty() == false){
        if((fd = open(opts.input_file_path.c_str(), O_RDONLY)) <0  ){
            davix_errno_to_davix_error(errno, scope, std::string("for source file ").append(opts.input_file_path), err);
            return -1;
        }
    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;
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
    std::fill(res, res+10, '-');
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
    }else {
        if(count > 0)
            std::cout << "Authentication Failure, try again:\n";
        else
            std::cout << "Authentication needed:\n";
        if( ask_user_login(login) > 0){
            if(ask_user_passwd(password) > 0){
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
