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
#include <cstdio>
#include <fstream>

// @author : Devresse Adrien
// main file for davix low level cmd line tool


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_main = "Davix::Tools::davix-rm";

const std::string openTag_req = "<Delete>";
const std::string closeTag_req = "</Delete>";
const std::string openTags_entry = "<Object><Key>";
const std::string closeTags_entry = "</Key></Object>";

std::string  get_base_rm_options(){
    return "  Delete Options:\n"
           "\t-r NUMBER_OF_THREADS:     Remove contents recursively, also works on S3 objects for storage systems that supports multi-objects deletion.\n"
           "\t-n NUMBER_OF_KEYS:        Number of object keys to include in each delete request, S3 supports up to 1000, but request will likely timeout. Default: 20\n";
}
static std::string help_msg(const std::string & cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_base_description_options();
    help_msg += Tool::get_common_options();
    help_msg += get_base_rm_options();

    return help_msg;
}

static int populateTaskQueue(Context& c, const Tool::OptParams & opts, std::string uri, DavixTaskQueue* tq, DavixError** err ){
    DAVIX_DIR* fd = NULL;
    DavPosix pos(&c);
    DavixError* tmp_err=NULL;
    struct stat st;
    struct dirent* d;
    int counter = 0;
    int entry_counter = 0;
    std::string protocol, host;
    std::string buffer(openTag_req);

    if( (fd = pos.opendirpp(&opts.params, opts.vec_arg[0], &tmp_err)) == NULL){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

    if(opts.vec_arg[0].compare(2,1,"s") == 0){
        protocol = "s3s://";
    }
    else{
        protocol = "s3://";
    }

    host = protocol + Uri(uri).getHost();

    while( (d = pos.readdirpp(fd, &st, &tmp_err)) != NULL ){
        // for every entry

        //TODO: unless user has turned on rr mode, skip entries that end with a '/'?
        //that way we can provide a way to just delete files inside the directory and not the other directories

        // format to xml and push to buffer string, for every n entries, create a DeleteOp(PostRequest)
        std::string entry(d->d_name);

        // for each entry, <Object><Key>entry</Key></Object>
        buffer += openTags_entry + entry + closeTags_entry;
        counter++;
        entry_counter++;

        // although the S3 API supports up to 1000 keys per request, in practice this will most likely results in a 504 gateway timeout
        if(counter == opts.s3_delete_per_request){ // default is set to 20, just to be safe
            buffer += closeTag_req;


            // push op to task queue
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to work queue, target is {}.", host);

            DeleteOp* op = new DeleteOp(opts, host, c, buffer);
            tq->pushOp(op);

            counter = 0;
            buffer.clear();
            buffer = openTag_req;
        }

        if(!opts.debug)
                Tool::batchTransferMonitor(uri, "Multi-objects delete ", entry_counter, 0);
    }
    // check if there is still anything in buffer for remainder entries
    if(!buffer.empty()){
        buffer += closeTag_req;

        // push op to task queue
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to work queue, target is {}.", host);

        DeleteOp* op = new DeleteOp(opts, host, c, buffer);
        tq->pushOp(op);
    }

    pos.closedirpp(fd, NULL);
    return 0;
}


static int preDeleteCheck(Tool::OptParams & opts, DavixError** err ) {
    Context c;
    configureContext(c, opts);
    DavPosix f(&c);
    struct stat st;
    DavixError* tmp_err=NULL;

    // see if the requested url actually points to a directory or not
    int r = f.stat(&opts.params, opts.vec_arg[0], &st, &tmp_err);
    if (tmp_err) {
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    if (r && !(st.st_mode & S_IFDIR)){
        DavixError::setupError(&tmp_err, scope_main, StatusCode::IsNotADirectory, "Failed to delete, target is not a directory.");
        DavixError::propagateError(err, tmp_err);
        return -1;
    }

    DavixTaskQueue tq;

    // create threadpool instance
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Creating threadpool");
    DavixThreadPool tp(&tq, opts.threadpool_size);

    // TODO two modes, one to delete "recursively, one for depth of 1. can probbaly do it in readdirpp

    // set listing mode to SemiHierarchical, we want to get every object under the same prefix in one go
    opts.params.setS3ListingMode(S3ListingMode::SemiHierarchical);
    // unfortunately s3 defaults max-keys to 1000 and doesn't provide a way to disable the cap, set to large number
    opts.params.setS3MaxKey(999999999);

    std::string url(opts.vec_arg[0]);

    if (url[url.size()-1] != '/')
        url += '/';

    populateTaskQueue(c, opts, url, &tq, &tmp_err);

    // if task queue is empty, then all work is done, stop workers. Otherwise wait.
    while(!tq.isEmpty()){
        sleep(2);
    }
    tp.shutdown();
    Tool::flushFinalLineShell(STDOUT_FILENO);

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

    if( (retcode= Tool::parse_davix_rm_options(argc, argv, opts, &tmp_err)) ==0){
        if( (retcode = Tool::configureAuth(opts)) == 0){
            if(checkProtocolSanity(opts, opts.vec_arg[0], &tmp_err)) {
                if ( opts.params.getAzureKey().size() != 0) {
                    opts.params.setProtocol(RequestProtocol::Azure);
                }else if (opts.vec_arg[0].compare(0,4,"http") ==0){
                    opts.params.setProtocol(RequestProtocol::Http);
                }else if( opts.vec_arg[0].compare(0,2,"s3") ==0){
                    opts.params.setProtocol(RequestProtocol::AwsS3);
                }else if ( opts.vec_arg[0].compare(0, 3,"dav") ==0){
                    opts.params.setProtocol(RequestProtocol::Webdav);
                }

                // only s3 needs special treatment, WebDAV delete on collection already works
                if((opts.params.getProtocol() == RequestProtocol::AwsS3) && (opts.params.getRecursiveMode())){
                    preDeleteCheck(opts, &tmp_err);
                }
                else{
                    Context c;
                    configureContext(c, opts);

                    DavFile f(c,opts.vec_arg[0]);
                    f.deletion(&opts.params, &tmp_err);
                }
                if(tmp_err != NULL) retcode = 1;
            }
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}
