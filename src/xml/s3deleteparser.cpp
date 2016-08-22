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

#include "s3deleteparser.hpp"
#include <utils/davix_s3_utils.hpp>

#include <stack>
#include <utils/davix_logger_internal.hpp>


namespace Davix{

const std::string delete_result_prop = "DeleteResult";
const std::string delete_prop = "Deleted";
const std::string key_prop = "Key";
const std::string error_prop = "Error";
const std::string code_prop = "Code";
const std::string message_prop = "Message";

struct S3DeleteParser::Internal{
    std::string current;
    std::string prefix;
    int entry_count;
    std::stack<std::string> stack_status;
    std::deque<FileDeleteStatus> del_status;
    FileDeleteStatus status;
    std::deque<FileProperties> props; // not used

    int start_elem(const std::string &elem){
        // new tag, clean content;
        current.clear();

        // check XML nested level, security protection
        if(stack_status.size() < 200){
            stack_status.push(elem);
        }else{
            throw DavixException(davix_scope_xml_parser(), StatusCode::ParsingError, "Impossible to parse S3 content, corrupted XML");
        }

        // check element, if it is "deleted" this recource has beed deleted successfully
        // or the resource did not exist in the first place, either way, log it
        if( StrUtil::compare_ncase(delete_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "deleted entry found", elem.c_str());
            status.clear();
            entry_count = 0;
        }

        // check element, if "Error" there has been problem with deleting this resource
        // the code returned will have to be mapped to http code
        if( StrUtil::compare_ncase(error_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "error entry found", elem.c_str());
            status.clear();
            status.error = true;
            entry_count = 0;
        }


        return 1;
    }

    int add_chunk(const std::string & chunk){
        current.append(chunk);
        return 0;
    }

    int end_elem(const std::string &elem){
        StrUtil::trim(current);

        // if "Key", current is file name
        // if "Code", current is error code
        // if "Message", current is error message

        // if "Delete", end of successful delete entry for that resource, push it
        if( StrUtil::compare_ncase(delete_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push deleted status for {}", status.filename.c_str());
            del_status.push_back(status);
            entry_count++;
        }


        // if "Error", end of error entry for that resource, push it
        if( StrUtil::compare_ncase(error_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push error status for {}", status.filename.c_str());
            del_status.push_back(status);
            entry_count++;
        }

        // if "Key", current is file name
        if( StrUtil::compare_ncase(key_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "key found for {}", current);
            status.filename = current;
        }

        // if "Code", current is error code
        if( StrUtil::compare_ncase(code_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "code found {}", current);
            status.error_code = current;
        }

        // if "Message", current is error message
        if( StrUtil::compare_ncase(message_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "error message found {}", current);
            status.message = current;
        }

        // reduce stack size
        if(stack_status.size() > 0)
            stack_status.pop();
        current.clear();
        return 0;
    }

};

S3DeleteParser::S3DeleteParser() : d_ptr(new Internal())
{
}

S3DeleteParser::~S3DeleteParser(){
}


int S3DeleteParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) nspace;
    (void) atts;
    return d_ptr->start_elem(std::string(name));

}

int S3DeleteParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    (void) len;
    return d_ptr->add_chunk(std::string(cdata, len));
}

int S3DeleteParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    return d_ptr->end_elem(std::string(name));
}

std::deque<FileDeleteStatus> & S3DeleteParser::getDeleteStatus(){
    return d_ptr->del_status;
}

// not used
std::deque<FileProperties> & S3DeleteParser::getProperties(){
    return d_ptr->props;
}

}
