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


// @author : Devresse Adrien
// main file for davix-get operation


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_get = "Davix::Tools::davix-get";



static std::string help_msg(){
    return Tool::get_base_description_options() +
           Tool::get_common_options()+ "\n";
}


static int execute_get(const Tool::OptParams & opts, int out_fd, DavixError** err){
        Context c;
        configureContext(c, opts);
        DavFile f(c, opts.vec_arg[0]);
        return f.getToFd(&opts.params, out_fd, err);
}



int get_output_get_fstream(const Tool::OptParams & opts,  const std::string & scope, DavixError** err){
    int fd = -1;
    Uri origin(opts.vec_arg[0]);

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


int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg();
    int out_fd= -1;

    if( (retcode= Tool::parse_davix_get_options(argc, argv, opts, &tmp_err)) ==0
        && (retcode = Tool::configureAuth(opts, &tmp_err)) == 0){

        if( ( out_fd = get_output_get_fstream(opts, scope_get, &tmp_err)) > 0){
            retcode = execute_get(opts, out_fd, &tmp_err);
            Tool::flushFinalLineShell(out_fd);
            close(out_fd);
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}







