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
#include <tools/davix_tool_params.hpp>
#include <tools/davix_tool_util.hpp>
#include <cstdio>

// @author : Devresse Adrien
// main file for davix-put operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_put = "Davix::Tools::davix-put";



static std::string help_msg(){
    return Tool::get_put_description_options() +
           Tool::get_common_options()+ "\n";
}


static int execute_put(const Tool::OptParams & opts, int fd, DavixError** err){
        const std::string &  src_file = opts.input_file_path;
        const std::string &  dst_file = opts.vec_arg[1];
        Context c;
        configureContext(c, opts);
        DavFile f(c, dst_file);
        struct stat st;
        if( fstat(fd, &st) != 0){
            davix_errno_to_davix_error(errno, scope_put, std::string("for source file ").append(src_file), err);
            return -1;
        }
        if( S_ISREG(st.st_mode) && st.st_size > 0){
            return f.putFromFd(&opts.params, fd, static_cast<dav_size_t>(st.st_size), err);
        }
        DavixError::setupError(err, scope_put, StatusCode::SystemError, std::string(dst_file).append("is not a valid regular file"));
        return -1;
}




int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();
    int fd_in= -1;

    if( (retcode= Tool::parse_davix_put_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::configureAuth(opts)) == 0){
        if( ( fd_in = Tool::getInFd(opts, scope_put, &tmp_err)) > 0){
            retcode = execute_put(opts, fd_in, &tmp_err);
            close(fd_in);
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}







