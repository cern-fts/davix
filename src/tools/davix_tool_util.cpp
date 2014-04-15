/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix.hpp>
#include <davix_internal.hpp>
#include <string_utils/stringutils.hpp>
#include "davix_tool_util.hpp"


#include <simple_getpass/simple_get_pass.h>


namespace Davix{

namespace Tool{

using namespace StrUtil;

dav_ssize_t writeToFd(int fd, const void* buffer, dav_size_t s_buff){
    while(1){
        ssize_t ret;
        errno =0;
        if((ret= write(fd, buffer, static_cast<size_t>(s_buff))) <0 && errno == EINTR){
            continue;
        }
        return static_cast<dav_ssize_t>(ret);
    }
}

dav_ssize_t writeToFd(int fd, const std::string & str){
    return writeToFd(fd, str.c_str(), str.size());
}

std::string string_from_ptime(const time_t & t){
    char b[255];
    b[sizeof(b)-1]= '\0';

    struct tm local_time={0};
    (void) localtime_r(&t, &local_time);
    strftime(b, 254, "%F %T", &local_time);
    return std::string(b);
}



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

void writeConsoleLine(int fd, char symbol,  const std::string & msg){
    std::ostringstream ss;
    ss << symbol << " " << msg << "\n";
    writeToFd(fd, ss.str());
}

int configureAuth(OptParams & opts, DavixError** err){
    // setup client side credential
    opts.params.setClientCertCallbackX509(&authCallbackCert, &opts);
    // setup client login / password
    opts.params.setClientLoginPasswordCallback(&authCallbackLoginPassword, &opts);

    //setup aws creds
    if(opts.aws_auth.first.empty() == false){
        opts.params.setAwsAuthorizationKeys(opts.aws_auth.first, opts.aws_auth.second);
        opts.params.setProtocol(RequestProtocol::AwsS3);
    }

    return 0;
}


static void printHookHeaders(char symbol, const std::string & first_msg, const std::string & start_line){
    std::string req_header(start_line);
    StrUtil::remove(req_header, '\r');
    rtrim(req_header, StrUtil::isCrLf);
    std::vector<std::string> res;
    split(req_header, '\n', res);

    writeConsoleLine(STDOUT_FILENO, '*', first_msg);
    for(stringVec::iterator it =res.begin(); it != res.end(); ++it){
          writeConsoleLine(STDOUT_FILENO, symbol, *it);
    }
}

void hook_davix_tool_pre_send(HttpRequest& req, const std::string & start_line, void* userdata){
    printHookHeaders('>', "Send request", start_line);
}

void hook_davix_tool_pre_rec(HttpRequest& req, const std::string & start_line, void* userdata){
    printHookHeaders('<', "Receive answer", start_line);
}


void configureContext(Context &context, const OptParams & opts){
    if(opts.pres_flag & DISPLAY_HEADERS){
        context.setHookById(DAVIX_HOOK_REQUEST_PRE_SEND, (void*) hook_davix_tool_pre_send, NULL);
        context.setHookById(DAVIX_HOOK_REQUEST_PRE_RECVE, (void*)hook_davix_tool_pre_rec, NULL);
    }
}

int getOutFd(const Tool::OptParams & opts, const std::string & scope, DavixError** err){
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

int getInFd(const Tool::OptParams & opts, const std::string & scope, DavixError** err){
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


void errorPrint(DavixError ** err){
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




int authCallbackLoginPassword(void* userdata, const SessionInfo & info, std::string & login, std::string & password,
                                        int count, DavixError** err){
    OptParams* opts = static_cast<OptParams*>(userdata);
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

int authCallbackCert(void* userdata, const SessionInfo & info, X509Credential* cert, DavixError** err){
    OptParams* opts = static_cast<OptParams*>(userdata);
    int ret = -1;
    const std::string key_path(opts->priv_key), cred_path(opts->cred_path);


    if(cred_path.empty() == false){
        if( cert->loadFromFilePEM( ((key_path.empty()== false)?(key_path):(cred_path)),
                                  cred_path,
                                  "",
                                  err) <0){

            // FIX IT : Neon SSL API does not allow to know if error is bad path  or wrong password, try again with password
            std::string password;
            if( ask_user_passwd(password) <0
                    || cert->loadFromFilePEM(key_path, cred_path, password, err) <0 ){
                if(err && *err == NULL){
                    DavixError::setupError(err, "Davix::Tool::Auth",
                                           StatusCode::LoginPasswordError,
                                           "Impossible to use client credential");
                }
                return -1;
             }
        }
        std::cout << std::endl;
        return 0;
    }
    if(ret < 0)
        DavixError::setupError(err, "Davix::Tool::Auth",
                               StatusCode::LoginPasswordError,
                               "No valid client credential provided");
    return -1;

}


std::string string_from_mode(mode_t mode){
    const char* rmask ="xwr";
    std::string str(10,'-');

    str[0] = (S_ISDIR(mode))?'d':'-';
    for(size_t i=0; i < 9; ++i){
        str[9-i] = (( mode & (0x01 << i))?(rmask[i%3]):'-');
    }
    return str;
}


std::string string_from_size_t(size_t number, size_t size_string){
    unsigned int digit= static_cast<unsigned int>(log10(static_cast<double>(number)));
    std::ostringstream ss;
    ss << number;
    ss << std::string(((digit < size_string)?(size_string - digit):0), ' ');
    return ss.str();
}


std::string filename_from_uri(const std::string & current_dir, const Uri & uri){
    if(uri.getStatus() == StatusCode::OK){

    }
    return std::string();
}

bool isShell(int fd){
    if(isatty(fd) ==1)
        return true;
    errno =0;
    return false;
}

void flushFinalLineShell(int fd){
    if(isShell(fd)){
        writeToFd(fd, "\n", 1);
    }
}

}
}
