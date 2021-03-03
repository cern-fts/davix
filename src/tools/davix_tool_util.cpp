/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
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
#include <utils/stringutils.hpp>
#include "davix_tool_util.hpp"
#include <utils/davix_logger_internal.hpp>
#include <utils/davix_gcloud_utils.hpp>

#include <ctype.h>
#include "utils/simple_get_pass.h"

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
    (std::cerr << "Login: ").flush();
    std::cin.getline(l, 1023);
    login.assign(l);
    std::fill(l, l+1024,'\0');
    return login.size();
}


dav_ssize_t ask_user_passwd(std::string & passwd, const std::string &prompt = "Password: "){
    char p[1024] ={0};
    std::cerr << prompt;
    std::cerr.flush();
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
        opts.params.setAwsRegion(opts.aws_region);
        opts.params.setAwsToken(opts.aws_token);
        opts.params.setAwsAlternate(opts.aws_alternate);
        opts.params.setProtocol(RequestProtocol::AwsS3);
    }

    // setup azure creds
    if(opts.azure_key.empty() == false) {
        opts.params.setAzureKey(opts.azure_key);
        opts.params.setProtocol(RequestProtocol::Azure);
    }

    // setup gcloud creds
    if(opts.gcloud_creds_path.empty() == false) {
        gcloud::CredentialProvider provider;
        opts.params.setGcloudCredentials(provider.fromFile(opts.gcloud_creds_path));
        opts.params.setProtocol(RequestProtocol::Gcloud);
    }

    // setup swift creds
    if(opts.os_token.empty() == false && (opts.os_project_id.empty() == false || opts.swift_account.empty() == false)) {
        opts.params.setOSToken(opts.os_token);
        opts.params.setOSProjectID(opts.os_project_id);
        opts.params.setSwiftAccount(opts.swift_account);
        opts.params.setProtocol(RequestProtocol::Swift);
    }

    return 0;
}

static bool startswith(const std::string &str, const std::string &prefix) {
  if(prefix.size() > str.size()) return false;

  for(size_t i = 0; i < prefix.size(); i++) {
    if(str[i] != prefix[i]) return false;
  }
  return true;
}

const std::string scope_params = "Davix::Tools::Params";
bool checkProtocolSanity(OptParams &opts, const std::string &url, DavixError **err) {
    if(!opts.aws_auth.first.empty()) {
        if(!startswith(url, "s3://") && !startswith(url, "s3s://") && !startswith(url, "http://") && !startswith(url, "https://")) {
            DavixError::setupError(err, scope_params, StatusCode::InvalidArgument, fmt::format(" S3 credentials cannot be used with the protocol given in this URL : {}", url));
            return false;
        }
    }
    if(!opts.os_token.empty()) {
        if(!startswith(url, "swift://") && !startswith(url, "swifts://") && !startswith(url, "http://") && !startswith(url, "https://")) {
            DavixError::setupError(err, scope_params, StatusCode::InvalidArgument, fmt::format(" Swift credentials cannot be used with the protocol given in this URL : {}", url));
            return false;
        }
    }
    return true;
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

    // authentication based on command-line parameters?
    // never prompt for a password, even if provided credentials are wrong
    if(opts->userlogpasswd_through_cmdline) {
        login = opts->userlogpasswd.first;
        password = opts->userlogpasswd.second;
        return 0;
    }

    // keyboard-based authentication?
    // prompt again only if last attempt failed
    if(count == 0 && !opts->userlogpasswd.first.empty()) {
        login = opts->userlogpasswd.first;
        password = opts->userlogpasswd.second;
        return 0;
    }

    // need to provide credentials through keyboard
    if(count == 0) {
        std::cerr << "Basic authentication - server is asking for username and password:\n";
    }
    else {
        std::cerr << "Authentication failure, try again:\n";
    }

    if(ask_user_login(login) > 0) {
        if(ask_user_passwd(password) > 0) {
            std::cerr << std::endl;
            opts->userlogpasswd.first = login;
            opts->userlogpasswd.second = password;
            return 0;
        }
    }

    DavixError::setupError(err, "Davix::Tool::Auth",
                           StatusCode::LoginPasswordError,
                           "No valid login/password provided");
    return -1;
}

int authCallbackCert(void* userdata, const SessionInfo & info, X509Credential* cert, DavixError** err){
    (void) info;
    OptParams* opts = static_cast<OptParams*>(userdata);
    std::string key_path(opts->priv_key), cred_path(opts->cred_path);

    if(key_path.empty()) {
        key_path = cred_path;
    }

    // empty credentials?
    if(opts->cred_path.empty()) {
        if(err && *err == NULL){
            DavixError::setupError(err, "Davix::Tool::Auth",
                                   StatusCode::LoginPasswordError,
                                   "No valid client credential provided");
        }
        return -1;
    }

    // try with existing, cached password first
    if(cert->loadFromFilePEM(key_path, cred_path, opts->cred_pass, err) <0) {
        if( (*err)->getStatus() != StatusCode::CredDecryptionError || !opts->cred_pass.empty()) {
            // credential specific error
            std::cerr << "("<< (*err)->getErrScope() <<") Error: "<< (*err)->getErrMsg() << std::endl;
            DavixError::clearError(err);
            return -1;
        }
    }
    else {
        return 0;
    }

    // password is empty, prompt user
    DavixError::clearError(err);
    if(ask_user_passwd(opts->cred_pass, "Certificate password: ") < 0
        || cert->loadFromFilePEM(key_path, cred_path, opts->cred_pass, err) < 0) {

        if(err && *err) {
            std::cerr << "("<< (*err)->getErrScope() <<") Error: "<< (*err)->getErrMsg() << std::endl;
        }
        return -1;
    }
    std::cerr << std::endl;
    return 0;
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
    static bool print_header= false;
    static Chrono::TimePoint start_time, last_time;
    Chrono::TimePoint current_time;
    Chrono::Clock clk(Chrono::Clock::Monolitic, Chrono::Clock::Second);

    int out_fd = STDERR_FILENO; // output fd -> stderr

    if((op_type != tool_type)){
        return;
    }

    int progress = 0;
    dav_size_t baudrate =0;
    current_time = clk.now();

    if(print_header == false){
        std::string type = ((op_type == Transfer::Read) ?  "Read" : "Write");
        last_time = start_time = clk.now();

        std::cout << "Performing " << type << " operation on: " << url << std::endl;
        print_header = true;
    }else{
        // tty display is slow
        // display progression only every second
        if(current_time < (last_time + Chrono::Duration(1)) && (static_cast<dav_size_t>(bytes_transfered) != total_size))
            return;
    }
    last_time = clk.now();
    Chrono::Duration diff_time = last_time - start_time;
    if( diff_time.toTimeValue() > 0){
        baudrate = bytes_transfered/(diff_time.toTimeValue());
    }


    if(total_size > 0){
        progress = static_cast<int>((static_cast<double>(bytes_transfered) / static_cast<double>(total_size)) * 100);
    }

    printProgressBar(out_fd, progress, bytes_transfered, total_size, baudrate);
}


std::string normalize_unit(dav_size_t bytes){
    const char * tab_unit[] = { "", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei" };
    size_t size_tab_unit= sizeof(tab_unit)/sizeof(const char*);

    const dav_size_t factor = 10*1024;
    size_t i;

    for(i =0; i < size_tab_unit; ++i){
        if(bytes < factor){
            break;
        }
        bytes /= 1024;
    }
    return fmt::format("{}{}", bytes, tab_unit[i]);
}

void printProgressBar(int out_fd, int percent, dav_ssize_t bytes_transfered, dav_size_t total_size, dav_size_t baudrate){
    std::string bar;
    std::string unit;
    static bool printOnce = true;
    dav_size_t size = total_size;

    struct winsize win;
    ioctl(out_fd, TIOCGWINSZ, &win);


    // w = width of bar, r = 100/w
    double w = static_cast<double>(win.ws_row)/1.5;
    double r = 100/w;
    for(double i = 0; i < w; ++i){
        if(i < (static_cast<double>(percent)/r) ){
            bar.replace(i,1,"=");
        }else{
            bar.replace(i,1," ");
        }
    }

    if((percent == 100) && !printOnce)
        return;

    std::cout << "\r" "[" << bar << "] ";
    std::cout.width(3);
    std::cout << percent << "%     " <<
        normalize_unit(bytes_transfered) << 'B' << "/" << normalize_unit(size) << 'B';
    std::cout.width(10);
    std::cout << normalize_unit(baudrate) << "B/s" <<
        "                     " << std::flush;

    if(static_cast<dav_size_t>(bytes_transfered) == size){
        std::cout << std::endl;
        printOnce = false;
    }
}

int configureMonitorCB(OptParams & opts, Transfer::Type type){
    using namespace std;

    // active monitoring callback only if output != stdout and stderr is a terminal
    if(opts.output_file_path.empty() == false
       && isatty(STDERR_FILENO) ==1 ){
        TransferMonitorCB transMonCB;
        transMonCB = bind(&Tool::TransferMonitor, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, type);
        opts.params.setTransfertMonitorCb(transMonCB);
    }

    return 0;
}

void tokeniseUrl(std::string url, std::deque<std::string>& dirVec){
    if(url.empty())
        return;

    std::string::iterator iter;

    for(iter = url.begin(); iter != url.end(); ){
        std::string::iterator tmpIter = std::find(iter, url.end(), '/');
        std::string tmp = std::string(iter, tmpIter);
        dirVec.push_back(tmp);

        iter = tmpIter;
        if(tmpIter != url.end())
            ++iter;
    }
}

int mkdirP(std::string url, bool trim){

    int ret = -1;

    std::deque<std::string> dirVec;
    std::string::iterator iter;

    for(iter = url.begin(); iter != url.end(); ){
        std::string::iterator tmpIter = std::find(iter, url.end(), '/');
        std::string tmp = std::string(url.begin(), tmpIter);
        dirVec.push_back(tmp);

        iter = tmpIter;
        if(tmpIter != url.end())
            ++iter;
    }

    ret = mkdirP(dirVec, trim);

    return ret;
}

int mkdirP(std::deque<std::string>& dirVec, bool trim){
    std::deque<std::string> dirList;

    // ignore . entries, don't need to create
    if(dirVec[0] == "." || dirVec[0] == "./")
        dirVec.pop_front();

    // discard first and last entry, for cases where first already exists and last is a file to be create/open
    if(trim){
        dirVec.pop_front();
        dirVec.pop_back();
    }

   /*
    //TODO: optimise logic to skip tokens that have been created previously
    for(unsigned int i=0; i<dirVec.size(); ++i){
        if(mkdir(dirVec[i].c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
            if(errno == EEXIST)
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Failed to create local directory, {} already exist. Continuing", dirVec[i]);
            else{
                std::cout << std::endl << "Failed to create local directory for " << dirVec[i] << std::endl;
                return -1;
            }
        }
        return 0;
    }
    return 0;
    */
    if(dirVec.size() == 0)
        return 0;

    int ret = -1;

    // loop backwards and attempt to create dir with largest depth
    // break when an existing dir has been found, or one has been created successfully
    // failed attemps are pushed to queue to retry once we have the location of an existing dir
    for(int i=dirVec.size()-1; i>=0; --i){
        ret = mkdir(dirVec[i].c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if((ret == 0) || (errno == EEXIST)){
            break;
        }
        else{
            std::string tmp = dirVec[i];
            dirList.push_front(tmp);
        }
    }

    // at this point we only have to create the dir(s) they do not already exist
    for(unsigned int ii=0; ii<dirList.size(); ++ii){
        if(mkdir(dirList[ii].c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
            if(errno == EEXIST)
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Failed to create local directory, {} already exist. Continuing", dirList[ii]);
            else{
                std::cout << std::endl << "Failed to create local directory for " << dirList[ii] << std::endl;
                return -1;
            }
        }
    }
    return 0;
}


void batchTransferMonitor(std::string dirPath, std::string msg, int entryCount, int totalEntry){
    if(isatty(STDERR_FILENO) !=1)
        return;

    static bool print_header = false;
    static Chrono::TimePoint start_time, last_time;
    Chrono::TimePoint current_time;
    Chrono::Clock clk(Chrono::Clock::Monolitic, Chrono::Clock::Second);

    current_time = clk.now();


    if(print_header == false){
        last_time = start_time = clk.now();
        print_header = true;
    }else{
        if(current_time < (last_time + Chrono::Duration(1)))
            return;
    }

    std::cout << "\r" << msg << " " << dirPath <<
        "      Files processed: ";
    std::cout.width(10);
    std::cout << entryCount;

    if(totalEntry != 0)
        std::cout << "/" << totalEntry;

    std::cout << "                     " << std::flush;
}


} //Tool
} //Davix
