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
#include <cstdio>


// @author : Devresse Adrien
// main file for davix low level cmd line tool


using namespace Davix;
using namespace std;

#define READ_BLOCK_SIZE 4096


const std::string scope_main = "Davix::Tools::davix";


static void configureRequest(HttpRequest& req, Tool::OptParams & opts){
    req.setParameters(opts.params);
    if(opts.req_type.empty() == false)
        req.setRequestMethod(opts.req_type);

    req.setRequestBody(opts.req_content);

}

static std::string get_request_options(){
    return "  Request Options: \n"
    "\t--data                    Content of the request\n"
    "\t--request, -X:            Request operation to use (ex : GET, PUT, PROPFIND, etc..)\n"
    "\n"
                ;
}


static std::string help_msg(const std::string & cmd_path){
    std::string help_msg = fmt::format("Usage : {} ", cmd_path);
    help_msg += Tool::get_base_description_options();
    help_msg += Tool::get_common_options();
    help_msg += get_request_options();

    return help_msg;
}


static int read_stream(HttpRequest* req, int fd_out, DavixError** err){

    if(req->beginRequest(err) <0
       ||  req->readToFd(fd_out, err) < 0)
        return -1;
   return req->endRequest(err);
}

int main(int argc, char** argv){
    int retcode=-1;
    Tool::OptParams opts;
    DavixError* tmp_err=NULL;
    opts.help_msg = help_msg(argv[0]);
    int out_fd;

    if( (retcode= Tool::parse_davix_options(argc, argv, opts, &tmp_err)) ==0){
        Context c;
        configureContext(c, opts);
        if( (out_fd= Tool::getOutFd(opts, scope_main, &tmp_err)) > 0
            && (retcode = Tool::configureAuth(opts)) == 0){

            HttpRequest req(c, opts.vec_arg[0], &tmp_err);
            if( tmp_err == NULL){
                configureRequest(req, opts);
                retcode= read_stream(&req, out_fd, &tmp_err);
                Tool::flushFinalLineShell(out_fd);
                close(out_fd);
            }else{
                retcode =-1;
            }
        }
    }
    Tool::errorPrint(&tmp_err);
    return retcode;
}







