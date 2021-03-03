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

#ifndef DAVIX_TOOL_PARAMS_HPP
#define DAVIX_TOOL_PARAMS_HPP


#include <vector>
#include <string>
#include <davix.hpp>

namespace Davix{

namespace Tool{

typedef std::pair<std::string, std::string> HeaderParam;
typedef std::pair<std::string, std::string> LoginPasswd;
typedef std::vector<HeaderParam> HeaderVec;

// pres flag
#define LONG_LISTING_FLAG   0x01
#define DISPLAY_HEADERS     0x02

struct OptParams{
    OptParams();
    RequestParams params;
    // vector of non-option arguments in order
    std::vector<std::string> vec_arg;
    int verbose;
    int debug;
    int s3_delete_per_request;
    int threadpool_size;
    // request command
    std::string req_type;
    // help msg
    std::string help_msg;
    // credential path
    std::string cred_path;
    // priv key path
    std::string priv_key;
    // certificate decryption password
    std::string cred_pass;
    // output file -o
    std::string output_file_path;
    // input file path
    std::string input_file_path;
    // user  login/passwd
    LoginPasswd userlogpasswd;
    bool userlogpasswd_through_cmdline;
    // request content
    std::string req_content;
    // s3 auth
    std::pair<std::string, std::string> aws_auth;
    // s3 region
    std::string aws_region;
    // s3 security token
    std::string aws_token;
    // s3 alternate url
    bool aws_alternate;
    // azure key
    std::string azure_key;
    // gcloud credentials path
    std::string gcloud_creds_path;
    // openstack token for swift
    std::string os_token;
    // openstack project id for swift
    std::string os_project_id;
    // swift account
    std::string swift_account;
    // presentation flag
    int pres_flag;
    // shell flags
    int shell_flag;
    // input file flag
    bool has_input_file;
    // disable cap on task queue's size
    bool no_cap;
    // modules list
    std::vector<std::string> modules_list;
};

int parse_davix_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_ls_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_get_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_put_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_copy_options(int argc, char** argv, OptParams & p, DavixError** err);

int parse_davix_rm_options(int argc, char** argv, OptParams & p, DavixError** err);

std::string get_common_options();

std::string get_base_description_options();

std::string get_get_description_options();

std::string get_put_description_options();

std::string get_copy_description_options();
}

}

#endif // DAVIX_TOOL_PARAMS_HPP
