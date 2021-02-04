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

#include <davix.hpp>
#include <davix_internal.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <tools/davix_taskqueue.hpp>
#include <tools/davix_op.hpp>
#include <tools/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>
#include <cstdio>


// @author : Devresse Adrien
// main file for davix-put operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_put = "Davix::Tools::davix-put";

std::string  get_base_put_options(){
    return "  Put Options:\n"
           "\t-r NUMBER_OF_THREADS:     Upload directories and their contents recursively\n"
           "\t--no-100-continue         Never ask for a 100-Continue from the server (some do not support it)\n";
}

static std::string help_msg(const std::string & cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_put_description_options();
    help_msg += Tool::get_common_options();
    help_msg += get_base_put_options();

    return help_msg;
}


static int execute_put(Context& c, const Tool::OptParams & opts, int fd, DavixError** err){
        const std::string &  src_file = opts.input_file_path;
        const std::string &  dst_file = opts.vec_arg[1];

        TRY_DAVIX{
            DavFile f(c, dst_file);
            struct stat st;
            if( fstat(fd, &st) != 0){
                errno_to_davix_exception(errno, scope_put, std::string("for source file ").append(src_file));
            }
            if( S_ISREG(st.st_mode) && (st.st_size >= 0)){
                    f.put(&opts.params, fd, static_cast<dav_size_t>(st.st_size));
                    return 0;
            }
            throw DavixException(scope_put, StatusCode::SystemError, std::string(src_file).append("is not a valid regular file"));
      }CATCH_DAVIX(err);
      return -1;
}


static int tryMakeCollection(Context& c, const Tool::OptParams & opts, std::string dst_path){
    DavixError* err=NULL;
    DavFile f(c,dst_path);
    f.makeCollection(&opts.params, &err);

    if(err){
        if(err->getStatus() == StatusCode::FileExist){
            std::cerr << std::endl << scope_put << " " << dst_path << " already exists, continuing..." << std::endl;
            DavixError::clearError(&err);
        }
        else{
            Tool::errorPrint(&err);
            return -1;
        }
    }
    return 0;
}

static int populateTaskQueue(Context& c, const Tool::OptParams & opts, std::string src_path, std::string dst_path, DavixTaskQueue* tq, DavixError** err ){
    struct stat st;
    DIR* dp;
    struct dirent *de;
    int ret = -1;
    int entry_counter = 0;

    if((dp = opendir(src_path.c_str()) ) == NULL){
        DavixError* tmp_err=NULL;
        davix_errno_to_davix_error(errno, scope_put, std::string("for source file ").append(src_path), &tmp_err);
        Tool::errorPrint(&tmp_err);
        return -1;
    }

    // process entries recursively
    while ((de = readdir(dp)) != NULL) {
        if((strcmp(de->d_name, ".")) && (strcmp(de->d_name, ".."))){    // ignore "." and ".." entries
            if(stat(((src_path+de->d_name).c_str()), &st) == 0){    // check if entry is file or directory
                if(S_ISDIR(st.st_mode)){    // is directory
                    if(opts.params.getProtocol() != RequestProtocol::AwsS3){    // if protocol is S3, don't need make collection (not hierarchical)
                        ret = tryMakeCollection(c, opts, Uri::join(dst_path, de->d_name));
                        if(ret <0)
                            return ret;
                    }
                    ret = populateTaskQueue(c, opts, Uri::join(src_path, de->d_name)+"/", Uri::join(dst_path, de->d_name), tq, err);
                }
                else if(S_ISREG(st.st_mode) && st.st_size > 0){
                    //push op to task queue
                    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to work queue, source is {} and destination is {}.", src_path+de->d_name, dst_path+"/"+de->d_name);
                    PutOp* op = new PutOp(opts, Uri::join(src_path, de->d_name), Uri::join(dst_path, de->d_name), static_cast<dav_size_t>(st.st_size), c);
                    tq->pushOp(op);
                    entry_counter++;
                }
            }
            else{
                errno_to_davix_exception(errno, scope_put, std::string("for source file ").append(src_path));
                return -1;
            }
        }
        if(!opts.debug)
            Tool::batchTransferMonitor(src_path, "Populating task queue for", entry_counter, 0);
    }
    closedir(dp);
    return (err)?(-1):0;
}


static int prePutCheck(Tool::OptParams & opts, DavixError** err){
    struct stat st;
    int ret = -1;
    Context c;
    configureContext(c, opts);

    if((ret = stat(opts.input_file_path.c_str(), &st)) != 0){
        DavixError* tmp_err=NULL;
        davix_errno_to_davix_error(errno, scope_put, std::string("for source file ").append(opts.input_file_path), &tmp_err);
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

    if((st.st_mode & S_IFDIR) && opts.params.getRecursiveMode()){
        std::string src_path(opts.input_file_path);

        if(src_path[src_path.size()] != '/')
            src_path += '/';

        DavixTaskQueue tq;

        // create threadpool instance
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Creating threadpool");
        DavixThreadPool tp(&tq, opts.threadpool_size);

        if(opts.params.getProtocol() != RequestProtocol::AwsS3 || opts.params.getProtocol() != RequestProtocol::Swift){    // if protocol is S3 or Swift, don't need make collection (not hierarchical)
            ret = tryMakeCollection(c, opts, opts.vec_arg[1]);
            if(ret <0)
                return ret;
        }

        std::string url(opts.vec_arg[1]);
        if (url[url.size()-1] == '/')
            url.erase(url.size() - 1);

        ret = populateTaskQueue(c, opts, src_path, url, &tq, err);

        // if task queue is empty, then all work is done, stop workers. Otherwise wait.
        while(!tq.isEmpty()){
            sleep(2);
        }
        tp.shutdown();
        Tool::flushFinalLineShell(STDOUT_FILENO);
    }
    else{ // single file to upload, process it normally
        int fd_in = -1;
        if(((fd_in = Tool::getInFd(opts, scope_put, err)) > 0)
                && (Tool::configureMonitorCB(opts, Transfer::Write)) == 0){
            ret = execute_put(c, opts, fd_in, err);
            close(fd_in);
        }
    }
    return ret;
}


int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);

    if(( (retcode = Tool::parse_davix_put_options(argc, argv, opts, &tmp_err)) == 0)
        && ((retcode = Tool::configureAuth(opts)) == 0) && checkProtocolSanity(opts, opts.vec_arg[1], &tmp_err)){
        retcode = prePutCheck(opts, &tmp_err);
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}
