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

#include <sstream>
#include <davix.hpp>
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>


// @author : Devresse Adrien
// main file for davix ls tool


using namespace Davix;
using namespace std;


std::string  get_base_listing_options(){
    return "  Listing Options:\n"
           "\t-l --long-list:                  long Listing mode\n";
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
    ss << Tool::string_from_ptime(st->st_ctime) << " ";
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
            display_long_file_entry(opts.vec_arg[0], &st, opts, filestream);
        }else{
            display_file_entry(opts.vec_arg[0], opts, filestream);
        }
        return 0;
    }
    return -1;
}




int main(int argc, char** argv){
    int retcode;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);
    FILE* fstream = stdout;

    if( (retcode= Tool::parse_davix_ls_options(argc, argv, opts, &tmp_err)) ==0){
        if( (retcode = Tool::configureAuth(opts)) == 0){
            retcode = listing(opts, fstream, &tmp_err);
            if(retcode < 0 && tmp_err->getStatus() == StatusCode::IsNotADirectory){
                DavixError::clearError(&tmp_err);
                retcode = get_info(opts, fstream, &tmp_err);
            }
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}







