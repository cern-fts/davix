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


#include <utils/davix_types.hpp>
#include <davix.hpp>
#include <davix_internal.hpp>
#include <string_utils/stringutils.hpp>
#include "davix_tool_util.hpp"

#include <ctype.h>
#include <simple_getpass/simple_get_pass.h>

#include <sys/ioctl.h>
#include <unistd.h>

namespace Davix{

namespace Tool{

using namespace StrUtil;
using namespace std::placeholders;

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

    struct tm local_time;
    (void) localtime_r(&t, &local_time);
    strftime(b, 254, "%F %T", &local_time);
    return std::string(b);
}



dav_ssize_t ask_user_login(std::string & login){
    char l[1024] ={0};
    (std::cout << "Login: ").flush();
    std::cin.getline(l, 1023);
    login.assign(l);
    std::fill(l, l+1024,'\0');
    return login.size();
}


dav_ssize_t ask_user_passwd(std::string & passwd){
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

int configureAuth(OptParams & opts){
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
    rtrim(req_header, StrUtil::isCrLf());
    std::vector<std::string> res;
    split(req_header, '\n', res);

    writeConsoleLine(STDOUT_FILENO, '*', first_msg);
    for(stringVec::iterator it =res.begin(); it != res.end(); ++it){
          writeConsoleLine(STDOUT_FILENO, symbol, *it);
    }
}

static void printHookHeadersVec(char symbol, const std::string & first_msg, const std::string & init_line,
                                const HeaderVec & headers){
    std::string parsed_first_line(init_line);
    StrUtil::remove(parsed_first_line, '\r');
    rtrim(parsed_first_line, StrUtil::isCrLf());

    writeConsoleLine(STDOUT_FILENO, '*', first_msg);
    writeConsoleLine(STDOUT_FILENO, symbol, parsed_first_line);
    for(HeaderVec::const_iterator it =headers.begin(); it != headers.end(); ++it){
          std::string header_line(it->first);
          header_line.append(": ").append(it->second);
          writeConsoleLine(STDOUT_FILENO, symbol, header_line);
    }
}

void hook_davix_tool_pre_send(HttpRequest& req, const std::string & start_line){
    (void) req;
    printHookHeaders('>', "Send request", start_line);
}

void hook_davix_tool_pre_rec(HttpRequest& req, const std::string & init_line, const HeaderVec & headers, int status_code){
    (void) req;
    (void) status_code;
    printHookHeadersVec('<', "Receive answer", init_line, headers);
}


void configureContext(Context &context, const OptParams & opts){
    if(opts.pres_flag & DISPLAY_HEADERS){
        RequestPreSendHook send_hook(hook_davix_tool_pre_send);
        RequestPreReceHook rece_hook(hook_davix_tool_pre_rec);
        context.setHook(send_hook);
        context.setHook(rece_hook);
    }
    for(std::vector<std::string>::const_iterator it = opts.modules_list.begin(); it != opts.modules_list.end(); it++){
        context.loadModule(*it);
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
    (void) info;
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
    (void) info;
    OptParams* opts = static_cast<OptParams*>(userdata);
    const std::string key_path(opts->priv_key), cred_path(opts->cred_path);

    if(cred_path.empty() == false){
        // try without password
        if(  cert->loadFromFilePEM( ((key_path.empty()== false)?(key_path):(cred_path)),
                                  cred_path,
                                  "",
                                  err) <0){
            
            if( (*err)->getStatus() != StatusCode::CredDecryptionError){
                // credential specific error
                return -1;
            }

            // retry with password
            std::cout << std::endl;
            DavixError::clearError(err);
            std::string password;
            if( ask_user_passwd(password) <0
                    || cert->loadFromFilePEM(key_path, cred_path, password, err) <0 ){
                if(err && *err == NULL){
                    DavixError::setupError(err, "Davix::Tool::Auth",
                                           StatusCode::CredDecryptionError,
                                           "Impossible to use and decrypt client credential");

                }
                return -1;
             }
        }
        std::cout << std::endl;
        return 0;
    }
    if(err && *err == NULL){
        DavixError::setupError(err, "Davix::Tool::Auth",
                               StatusCode::LoginPasswordError,
                               "No valid client credential provided");
    }
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

std::string SanitiseTildedPath(const char * path){
    if(path[0] == '~')
    {
        std::string newpath(path);
        newpath.erase(0, 1);
        const char * val = std::getenv("HOME");
        
        if(val != NULL){
            newpath.insert(0, val);
        }
        else{
            newpath.insert(0, "/");
        }
        return newpath;
    }   
    return std::string(path);
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}


void TransferMonitor(const Uri & url, Transfer::Type op_type, dav_ssize_t bytes_transfered, dav_size_t total_size, Transfer::Type tool_type){
    static bool printOnce = true;

    if(isatty(fileno(stdout)) && (op_type == tool_type)){
        int progress = 0;
        if(printOnce){
            std::string type;
            
            (op_type == Transfer::Type::Read) ? type = "Read" : type = "Write";

            std::cout << "Performing " << type << " operation on: " << url << std::endl;
            printOnce = false;
        }
        
        if(total_size == 0)
            progress = 0;
        else
            progress = (static_cast<float>(bytes_transfered) / static_cast<float>(total_size)) * 100;

        printProgressBar(progress, bytes_transfered, total_size);
    }
}

void printProgressBar(const int percent, dav_ssize_t bytes_transfered, dav_size_t total_size){
    std::string bar;
    std::string unit;
    static bool printOnce = true;
    static Chrono::TimePoint start_time = Chrono::Clock(Chrono::Clock::Monolitic).now();
    Chrono::TimePoint current_time = Chrono::Clock(Chrono::Clock::Monolitic).now();
    unsigned long diff_time = current_time.toTimestamp() - start_time.toTimestamp();
    int size = (total_size == 0) ? 0 : total_size;
    unsigned long baudrate = 0;


    struct winsize win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

    if(diff_time == 0)
        baudrate = 0;
    else
        baudrate = (bytes_transfered/diff_time);
    
    // w = width of bar, r = 100/w
    int w = (win.ws_row/1.5);    
    float r = 100/static_cast<float>(w);    
    for(int i = 0; i < w; ++i){
        if(i < (percent/r) ){
            bar.replace(i,1,"=");
        }else{
            bar.replace(i,1," ");
        }
    }

    if((total_size / 1024) >= (1024 * 1024 * 1024)){
        size /= (1024 * 1024 * 1024);
        bytes_transfered /= (1024 * 1024 * 1024);
        baudrate /= (1024 * 1024 * 1024);
        unit = "GB";
    }else if(total_size >= (1024 * 1024 * 1024)){
        size /= (1024 * 1024);
        bytes_transfered /= (1024 * 1024);
        baudrate /= (1024 * 1024);
        unit = "MB";
    }else if(total_size >= (1024 * 1024)){
        size /= 1024;
        bytes_transfered /= 1024;
        baudrate /= 1024;
        unit = "KB";
    }else{
        unit = "B";
    }
    if((percent == 100) && !printOnce)
        return;

    std::cout << "\r" "[" << bar << "] ";
    std::cout.width(3);
    std::cout << percent << "%     " << 
        bytes_transfered << "/" << size << unit;
    std::cout.width(10); 
    std::cout << baudrate << unit << "/s" <<
        "                     " << std::flush;

    if(bytes_transfered == size){
        std::cout << std::endl;
        printOnce = false;
    }
}

int configureMonitorCB(OptParams & opts, Transfer::Type type){
    using namespace std;
    TransferMonitorCB transMonCB;
    transMonCB = bind(&Tool::TransferMonitor, _1, _2, _3, _4, type);
    opts.params.setTransfertMonitorCb(transMonCB);

    return 0;
}


}
}
