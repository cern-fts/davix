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

#include <davix_internal.hpp>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <tools/davix_taskqueue.hpp>
#include <tools/davix_op.hpp>
#include <tools/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>


// @author : Devresse Adrien
// main file for davix-get operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";

std::string  get_base_get_options(){
    return "  Get Options:\n"
           "\t--accepted-retry:         Number of retries upon receiving 202-Accepted. default: 180\n"
           "\t--accepted-retry-delay:   Time in seconds to wait between 202-Accepted retries. default: 10\n"
           "\t-r NUMBER_OF_THREADS:     Get directories and their contents recursively.\n";
}

static std::string help_msg(const std::string &cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_get_description_options();
    help_msg += Tool::get_common_options();
    help_msg += get_base_get_options();

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

static int excute_get(Context& c, const Tool::OptParams & opts, int out_fd, std::string uri, DavixError** err){
        int ret;
        DavFile f(c, uri);
        ret = f.getToFd(&opts.params, out_fd, err);
        return (ret >= 0)?0:-1;
}

static int populateTaskQueue(Context& c, const Tool::OptParams & opts, std::string uri, DavixTaskQueue* tq, DavixTaskQueue* listing_tq, DavixError** err ){
    DAVIX_DIR* fd = NULL;
    DavPosix pos(&c);
    DavixError* tmp_err=NULL;
    std::string outputPath;
    struct stat st;
    struct dirent* d;
    int entry_counter=0;
    std::string last_success_entry;

    std::deque<std::pair<std::string,std::string> > dirQueue;
    std::deque<std::pair<std::string,std::string> > opQueue;

    outputPath = opts.output_file_path;

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
    else
        DavixError::setupError(&tmp_err, "Davix::Tool::Get", StatusCode::InvalidArgument, " target URL is empty.");

    if( (fd = pos.opendirpp(&opts.params, dirQueue.front().first, &tmp_err)) == NULL){
        Tool::errorPrint(&tmp_err);

        // if the "root" level cannot be found, there is nothing we can do, just quit
        return -1;
    }

    // dir opened successfully, create local dir
    Tool::mkdirP(dirQueue.front().second, false);

    //if S3, remove last token of url to accommodate duplicate token in d_name
    if (opts.params.getProtocol() == RequestProtocol::AwsS3){
        dirQueue.front().first = "s3://"+(Uri(dirQueue.front().first).getHost())+"/";
    }

    while( ((d = pos.readdirpp(fd, &st, &tmp_err)) != NULL)){    // if one entry inside a directory fails, the loop exits, the other entires are not processed

        last_success_entry = dirQueue.front().first+d->d_name;
        // for each entry, do a stat to see if it's a directory, if yes, push to dirQueue for further processing
        if(st.st_mode & S_IFDIR){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Directory entry found, pushing {}/ to dirQueue", dirQueue.front().first+d->d_name);
            dirQueue.push_back(std::make_pair(Uri::join(dirQueue.front().first, d->d_name)+"/", Uri::join(dirQueue.front().second, d->d_name)));
        }
        else if(!(st.st_mode & S_IFDIR)){
            //if we spend too long in here, server will likely close the connection mid-readdirpp, need to get all the entries quickly before processing them
            opQueue.push_back(std::make_pair(Uri::join(dirQueue.front().first, d->d_name), Uri::join(dirQueue.front().second, d->d_name)));
        }
        entry_counter++;
        if(!opts.debug)
            Tool::batchTransferMonitor(dirQueue.front().first, "Crawling", entry_counter, 0);
    } // while readdirpp

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        cerr << endl << "Error occurred during listing  " << dirQueue.front().first << " Number of entries processed in current directory: " << entry_counter << ". Continuing..."<< endl;
        cerr << endl << "Last successful entry is " << last_success_entry << endl;
    }

    entry_counter = 0;

    pos.closedirpp(fd, NULL);
    dirQueue.pop_front();

    int num_of_ops = opQueue.size();
    int num_listing_ops = dirQueue.size();

    // if endpoint is S3 then there is no need to crawl namespace recursively, since it's flat
    if (opts.params.getProtocol() != RequestProtocol::AwsS3){
        for(unsigned int i=0; i < dirQueue.size(); ++i){
            //push listing op to task queue
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (listing) work queue, target is {} and destination is {}.", dirQueue[i].first, dirQueue[i].second);

            entry_counter++;

            if(!opts.debug)
                Tool::batchTransferMonitor(dirQueue[i].first, "Populating (listing) task queue for", entry_counter, num_listing_ops);

            ListppOp* l_op = new ListppOp(opts, (dirQueue[i].first), (dirQueue[i].second), c, tq, listing_tq);
            listing_tq->pushOp(l_op);
        }
        entry_counter = 0;
    }

    for(unsigned int i=0; i < opQueue.size(); ++i){
        //push op to task queue
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (get) work queue, target is {} and destination is {}.", opQueue[i].first, opQueue[i].second);

        entry_counter++;

        if(!opts.debug)
            Tool::batchTransferMonitor(opQueue[i].first, "Populating (get) task queue for", entry_counter, num_of_ops);

        GetOp* op = new GetOp(opts, (opQueue[i].first), (opQueue[i].second), c);
        tq->pushOp(op);
    }

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;
}


static int preGetCheck(Tool::OptParams & opts, DavixError** err ) {
    Context c;
    configureContext(c, opts);
    File f(c, opts.vec_arg[0]);
    struct stat st;
    DavixError* tmp_err=NULL;


    // Try to understand what to do
    // If the URL is for plain HTTP then we don't want to stat
    bool justgetfile = true;

    if ( opts.vec_arg[0].compare(0, 4, "http") != 0 ) {
        int r = f.stat(&opts.params, &st, &tmp_err);
        if (r && tmp_err) {
            DavixError::propagateError(err, tmp_err);
            return -1;
        }

        if ( !r && (st.st_mode & S_IFDIR) && opts.params.getRecursiveMode()) justgetfile = false;
    }

    if (justgetfile) {
        // target resource is a file or request protocol is http, just get it normally
        int out_fd= -1;
        if( ( (out_fd = get_output_get_fstream(opts, scope_get, opts.output_file_path, &tmp_err)) > 0)
                  && (Tool::configureMonitorCB(opts, Transfer::Read)) == 0) {

            excute_get(c, opts, out_fd, opts.vec_arg[0], &tmp_err);
            Tool::flushFinalLineShell(out_fd);
            close(out_fd);
        }
    }
    else {

        std::string url(opts.vec_arg[0]);

        if (url[url.size()-1] != '/')
            url += '/';

        // if protocol is S3, set listing mode to SemiHierarchical, we want to get every object under the same prefix in one go
        if (opts.params.getProtocol() == RequestProtocol::AwsS3){
            opts.params.setS3ListingMode(S3ListingMode::SemiHierarchical);
            // unfortunately s3 defaults max-keys to 1000 and doesn't provide a way to disable the cap, set to large number
            opts.params.setS3MaxKey(999999999);
        }

        DavixTaskQueue tq;  // for get ops
        DavixTaskQueue listing_tq;  // for listing ops

        // create threadpool instance
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Creating threadpool");
        DavixThreadPool tp(&tq, opts.threadpool_size);
        DavixThreadPool listing_tp(&listing_tq, opts.threadpool_size);

        populateTaskQueue(c, opts, url, &tq, &listing_tq, &tmp_err);

        // if both task queues are empty, then all work is done, stop workers. Otherwise wait.
        do{
            sleep(2);
        }while(!tq.isEmpty() || !listing_tq.isEmpty() || !tp.allThreadsIdle() || !listing_tp.allThreadsIdle());

        listing_tp.shutdown();
        tp.shutdown();
        Tool::flushFinalLineShell(STDOUT_FILENO);
    }

    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;
}


int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);

    if( (retcode= Tool::parse_davix_get_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::configureAuth(opts)) == 0 && checkProtocolSanity(opts, opts.vec_arg[0], &tmp_err)){
            retcode = preGetCheck(opts, &tmp_err);
    }
    Tool::errorPrint(&tmp_err);

    return retcode;
}
