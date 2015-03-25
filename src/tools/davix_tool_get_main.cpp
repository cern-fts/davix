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


#include <davix_internal.hpp>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <utils/davix_taskqueue.hpp>
#include <utils/davix_op.hpp>
#include <utils/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>


// @author : Devresse Adrien
// main file for davix-get operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";


static std::string help_msg(const std::string &cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_get_description_options();
    help_msg += Tool::get_common_options();

    return help_msg;
}

int get_output_get_fstream(const Tool::OptParams & opts,  const std::string & scope, std::string uri, DavixError** err){
    int fd = -1;
    if(uri.empty() == false){
        if((fd = open(uri.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777)) <0  ){
            davix_errno_to_davix_error(errno, scope, std::string("for destination file ").append(opts.output_file_path), err);
            return -1;
        }
    }else{
        fd = dup(STDOUT_FILENO);
    }
    return fd;
}

static int excute_get(const Tool::OptParams & opts, int out_fd, std::string uri, DavixError** err){
        int ret;
        Context c;
        configureContext(c, opts);
        DavFile f(c, uri);
        ret = f.getToFd(&opts.params, out_fd, err);
        return (ret >= 0)?0:-1;
}

static int populateTaskQueue(const Tool::OptParams & opts, std::string uri, std::string outputPath, DavixTaskQueue* tq, DavixError** err ){
    DAVIX_DIR* fd = NULL;
    Context c;
    configureContext(c, opts);
    DavPosix pos(&c);
    DavixError* tmp_err=NULL;
    struct stat st;
    struct dirent* d;
    int entry_counter=0;

    std::deque<std::pair<std::string,std::string> > dirQueue;

    // set up first entry
    if(!uri.empty()){
        // if output path is not specified, default to current directory
        if(outputPath.empty()){
            std::deque<std::string> dirVec;
           
            std::string tmp = Uri(uri).getPath();
            if(tmp == "/"){ // no output path and no token in url to use as default, fallback to ./temp
                outputPath = "./temp";
            }
            else{
                Tool::tokeniseUrl(Uri(uri).getPath(), dirVec);
                outputPath = "./"+dirVec.back();
            }
        }
        dirQueue.push_back(std::make_pair(uri, outputPath));
    }

    while(!dirQueue.empty()){
        if( (fd = pos.opendirpp(&opts.params, dirQueue.front().first, &tmp_err)) == NULL){
            Tool::errorPrint(&tmp_err);
            return -1;
        }

        // dir opened successfully, create local dir
        Tool::mkdirP(dirQueue.front().second, false);

        //if S3, remove last token of url to accommodate duplicate token in d_name
        if (opts.params.getProtocol() == RequestProtocol::AwsS3){
            dirQueue.front().first = "s3://"+(Uri(dirQueue.front().first).getHost())+"/";
        } 

        while( ((d = pos.readdirpp(fd, &st, &tmp_err)) != NULL)){    // if one entry inside a directory fails, the loop exits, the other entires are not processed
            // for each entry, do a stat to see if it's a directory, if yes, push to dirQueue for further processing    
            if(st.st_mode & S_IFDIR){
                DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Directory entry found, pushing {}/ to dirQueue", dirQueue.front().first+d->d_name);
                dirQueue.push_back(std::make_pair(dirQueue.front().first+d->d_name+"/",dirQueue.front().second+"/"+d->d_name));
            }
            else if(!(st.st_mode & S_IFDIR)){
                //push op to task queue
                DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to work queue, target is {} and destination is {}.", dirQueue.front().first+d->d_name, dirQueue.front().second+"/"+d->d_name);
                GetOp* op = new GetOp(opts, (dirQueue.front().first+d->d_name), (dirQueue.front().second+"/"+d->d_name), &tmp_err);
                tq->pushOp(op);
                entry_counter++;
            }
            Tool::batchTransferMonitor(dirQueue.front().first, entry_counter);
        }
        
        if(tmp_err){
            Tool::errorPrint(&tmp_err);
            cerr << endl << "Error occured during listing  " << dirQueue.front().first << " Number of entries processed: " << entry_counter << ". Continuing..."<< endl;
        }

        entry_counter = 0;
        
        pos.closedirpp(fd, NULL);
        dirQueue.pop_front();
    }

    return (tmp_err && tmp_err)?(-1):0;
}

static int preGetCheck(Tool::OptParams & opts, DavixError** err ){
    Context c;
    configureContext(c, opts);
    File f(c, opts.vec_arg[0]);
    struct stat st;    
    int ret = -1;
    DavixError* tmp_err=NULL;

    // check target resource is a file or a collection
    if( f.stat(&opts.params, &st, &tmp_err) == 0){
        if(st.st_mode & S_IFDIR && (opts.params.getProtocol() != RequestProtocol::Http)){ // resource requested is a directory
            std::string url(opts.vec_arg[0]);

            if (url[url.size()] != '/')
                url += '/';
            
            DavixTaskQueue tq;

            // create threadpool instance 
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Creating threadpool");
            DavixThreadPool tp(&tq);
           
            // if protocol is S3, set listing mode to SemiHierarchical, we want to get every object under the same prefix in one go
            if (opts.params.getProtocol() == RequestProtocol::AwsS3){
                opts.params.setS3ListingMode(S3ListingMode::SemiHierarchical);
            }

            ret = populateTaskQueue(opts, url, opts.output_file_path, &tq, &tmp_err);
            
            // if task queue is empty, then all work is done, stop workers. Otherwise wait.
            while(!tq.isEmpty()){
                sleep(2);
            }
            tp.shutdown();
            Tool::flushFinalLineShell(STDOUT_FILENO);
        }
        else{ // target resource is a file or request protocol is http, just get it normally
            int out_fd= -1;
            if( ( (out_fd = get_output_get_fstream(opts, scope_get, opts.output_file_path, &tmp_err)) > 0)
                && (Tool::configureMonitorCB(opts, Transfer::Read)) == 0){
                ret = excute_get(opts, out_fd, opts.vec_arg[0], &tmp_err);
                Tool::flushFinalLineShell(out_fd);
                close(out_fd);
            }
        }
        if(tmp_err)
            DavixError::propagateError(err, tmp_err);

        return (ret >= 0)?0:-1;
    }
    if(tmp_err)
    DavixError::propagateError(err, tmp_err);
    return -1;
}

int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);

    if( (retcode= Tool::parse_davix_get_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::configureAuth(opts)) == 0){
            retcode = preGetCheck(opts, &tmp_err);
    }
    Tool::errorPrint(&tmp_err);

    return retcode;
}

