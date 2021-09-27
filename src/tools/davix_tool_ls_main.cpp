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

#include <sstream>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <tools/davix_taskqueue.hpp>
#include <tools/davix_op.hpp>
#include <tools/davix_thread_pool.hpp>
#include <utils/davix_logger_internal.hpp>
#include <pthread.h>

// @author : Devresse Adrien
// main file for davix ls tool


using namespace Davix;
using namespace std;


std::string  get_base_listing_options(){
    return "  Listing Options:\n"
           "\t--long-list, -l:          long Listing mode\n"
           "\t-r NUMBER_OF_THREADS      List directories's content recursively using multiple threads\n"
           "\t--no-cap:                 Disable size cap on task queue for pending listing operations\n"
           "\t--s3-listing:             S3 bucket listing mode - flat, semi or hierarchical(default)\n"
           "\t--s3-maxkeys:             Maximum number of entries returns by S3 list bucket request. default: 10000\n"
           "\t--swift-listing:          Swift listing mode - semi or hierarchical(default)\n";
}

static std::string help_msg(const std::string & cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_base_description_options();
    help_msg += Tool::get_common_options();
    help_msg += get_base_listing_options();

    return help_msg;
}


static void display_file_entry(const std::string & filename, const Tool::OptParams & opts, FILE* filestream){
    (void) opts;
    fputs(filename.c_str(), filestream);
    fputs("\n",filestream);
}


static void display_long_file_entry(const std::string & filename,  struct stat* st, const Tool::OptParams & opts, FILE* filestream){
    (void) opts;
    std::ostringstream ss;
    ss << Tool::string_from_mode(st->st_mode) << " ";
    ss << Tool::string_from_size_t(static_cast<size_t>(st->st_nlink),4) << " ";
    ss << Tool::string_from_size_t(st->st_size, 9) << " ";
    ss << Tool::string_from_ptime(st->st_mtime) << " ";
    ss << filename << "\n";

    fputs(ss.str().c_str(), filestream);
}

static int listing(const Tool::OptParams & opts, FILE* filestream, DavixError** err ){
    DAVIX_DIR* fd = NULL;
    Context c;
    configureContext(c, opts);
    DavPosix pos(&c);
    struct stat st;
    struct dirent* d;
    if( (fd = pos.opendirpp(&opts.params, opts.vec_arg[0], err)) == NULL)
        return -1;
    while( (d = pos.readdirpp(fd, &st, err)) != NULL ){
        if(opts.pres_flag & LONG_LISTING_FLAG){
            display_long_file_entry(d->d_name, &st, opts, filestream);
        }else{
            display_file_entry(d->d_name, opts, filestream);
        }
    }
    pos.closedirpp(fd, NULL);
    return (err && *err)?(-1):0;
}

static int get_info(const Tool::OptParams & opts, FILE* filestream, DavixError** err ){
    Context c;
    configureContext(c, opts);
    File f(c, opts.vec_arg[0]);
    struct stat st;
    if( f.stat(&opts.params, &st, err) == 0){
        if(opts.pres_flag & LONG_LISTING_FLAG){
            if( opts.params.getProtocol() == RequestProtocol::AwsS3){
                std::string tmp(opts.vec_arg[0]);
                display_long_file_entry(tmp.substr(tmp.rfind('/')+1), &st, opts, filestream);
            }
            else{
                display_long_file_entry(opts.vec_arg[0], &st, opts, filestream);
            }
        }else{
            display_file_entry(opts.vec_arg[0], opts, filestream);
        }
        return 0;
    }
    return -1;
}

static int populateTaskQueue(Context& c, const Tool::OptParams & opts, std::string uri, DavixTaskQueue* listing_tq, DavixError** err, FILE* filestream, pthread_mutex_t& output_mutex){
    DAVIX_DIR* fd = NULL;
    DavPosix pos(&c);
    DavixError* tmp_err=NULL;
    std::string outputPath;
    struct stat st;
    struct dirent* d;
    unsigned long entry_counter = 0;
    std::string last_success_entry;

    std::deque<std::string> dirQueue;

    // set up first entry
    if(!uri.empty()){
        dirQueue.push_back(uri);
    }
    else
        DavixError::setupError(&tmp_err, "Davix::ListOp", StatusCode::InvalidArgument, " target URL is empty.");

    if( (fd = pos.opendirpp(&opts.params, dirQueue.front(), &tmp_err)) == NULL){
        Tool::errorPrint(&tmp_err);
        return -1;
    }

    Uri tmp(uri);
    std::string fullpath = tmp.getPath();
    if(fullpath[0] == '/') fullpath.erase(0,1);

    while( ((d = pos.readdirpp(fd, &st, &tmp_err)) != NULL)){    // if one entry inside a directory fails, the loop exits, the other entires are not processed

        last_success_entry = dirQueue.front()+d->d_name;
        // for each entry, do a stat to see if it's a directory, if yes, push to dirQueue for further processing
        if(st.st_mode & S_IFDIR){
            DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Directory entry found, pushing {}/ to dirQueue", dirQueue.front()+d->d_name);
            dirQueue.push_back(dirQueue.front()+d->d_name+"/");
        }

        if(opts.pres_flag & LONG_LISTING_FLAG){
            display_long_file_entry(fullpath+d->d_name, &st, opts, filestream);
        }else{
            display_file_entry(fullpath+d->d_name, opts, filestream);
        }
    } // while readdirpp

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        std::cerr << std::endl << "Error occurred during listing  " << dirQueue.front() << " Number of entries processed in current directory: " << entry_counter << ". Continuing..."<< std::endl;
        std::cerr << std::endl << "Last successful entry is " << last_success_entry << std::endl;
    }

    pos.closedirpp(fd, NULL);
    dirQueue.pop_front();

    for(unsigned int i=0; i < dirQueue.size(); ++i){
        //push listing op to task queue
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Adding item to (listing) work queue, target is {}.", dirQueue[i]);
        ListOp* l_op = new ListOp(opts, dirQueue[i], c, listing_tq, filestream, output_mutex);
        listing_tq->pushOp(l_op);
    }

    if(tmp_err){
        Tool::errorPrint(&tmp_err);
        return -1;
    }
    return 0;
}

static int recursiveListing(const Tool::OptParams & opts, FILE* filestream, DavixError** err ){
    Context c;
    configureContext(c, opts);
    File f(c, opts.vec_arg[0]);
    DavixError* tmp_err=NULL;

    std::string url(opts.vec_arg[0]);

    if (url[url.size()-1] != '/')
        url += '/';

    DavixTaskQueue listing_tq(opts, QueueType::LIFO);  // for listing ops

    // init output mutex for child listOps, only one listOp can output to a stream at a given time
    pthread_mutex_t output_mutex;
    pthread_mutex_init(&output_mutex, NULL);

    // create threadpool instance
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_CORE, "Creating threadpool");
    DavixThreadPool listing_tp(&listing_tq, opts.threadpool_size);

    populateTaskQueue(c, opts, url, &listing_tq, &tmp_err, filestream, output_mutex);

    do{
        sleep(2);
    }while((!listing_tq.isEmpty()) || !(listing_tp.allThreadsIdle()));


    listing_tp.shutdown();
    Tool::flushFinalLineShell(STDOUT_FILENO);


    if(tmp_err){
        DavixError::propagateError(err, tmp_err);
        return -1;
    }
    return 0;

}

int main(int argc, char** argv){
    int retcode;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);
    FILE* fstream = stdout;

    if( (retcode= Tool::parse_davix_ls_options(argc, argv, opts, &tmp_err)) ==0){
        if( (retcode = Tool::configureAuth(opts)) == 0){
            if(checkProtocolSanity(opts, opts.vec_arg[0], &tmp_err)) {
              // if recursive and S3
              if(opts.params.getRecursiveMode() && (opts.vec_arg[0].compare(0,2,"s3") ==0)){
                  // don't need to use -r switch for S3, just set listing mode to SemiHierarchical and do a normal listing
                  opts.params.setS3ListingMode(S3ListingMode::SemiHierarchical);
                  // unfortunately s3 defaults max-keys to 1000 and doesn't provide a way to disable the cap, set to large number
                  opts.params.setS3MaxKey(999999999);

                  retcode = listing(opts, fstream, &tmp_err);
              }
              else if(opts.params.getRecursiveMode() && (opts.vec_arg[0].compare(0, 5, "swift")==0)) {
                  // don't need to use -r switch for Swift, just set listing mode to SemiHierarchical and do a normal listing
                  opts.params.setSwiftListingMode(SwiftListingMode::SemiHierarchical);
                  retcode = listing(opts, fstream, &tmp_err);
              }
              else if(opts.params.getRecursiveMode()){    // dav recursive listing
                  retcode = recursiveListing(opts, fstream, &tmp_err);
              }
              else{   // normal listing
                  retcode = listing(opts, fstream, &tmp_err);
              }
              // just a single file
              if(retcode < 0 && tmp_err->getStatus() == StatusCode::IsNotADirectory){
                  DavixError::clearError(&tmp_err);
                  retcode = get_info(opts, fstream, &tmp_err);
              }
            }
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}
