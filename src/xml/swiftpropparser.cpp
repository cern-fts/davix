/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2021
 * Author: Shiting Long <s.long@fz-juelich.de> (Forschungszentrum Juelich)
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

#include "swiftpropparser.hpp"
#include <utils/davix_swift_utils.hpp>

#include <stack>
#include <utils/davix_logger_internal.hpp>

namespace Davix {

const std::string col_prop = "Name";
const std::string delimiter_prop = "Contents";
const std::string name_prop = "Key";
const std::string size_prop = "Size";

const std::string prefix_prop = "Prefix";
const std::string com_prefix_prop = "CommonPrefixes";
const std::string listbucketresult_prop = "ListBucketResult";
const std::string last_modified_prop = "LastModified";


struct SwiftPropParser::Internal {
    std::string current;
    std::string name;
    std::string prefix_to_remove;
    std::deque<FileProperties> props;
    FileProperties property;

    int start_elem(const std::string &elem){
        // new tag, clean content;
        current.clear();

        // if new entry (a directory), clear entry
        if(StrUtil::compare_ncase("subdir", elem) ==0){
            property.clear();
        }

        // if new entry (an object), clear entry
        if(StrUtil::compare_ncase("object", elem) == 0) {
            property.clear();
        }

        // if a new container, clear entry
        if(StrUtil::compare_ncase("container", elem) == 0) {
            property.clear();
        }

        return 1;
    }

    int add_chunk(const std::string & chunk){
        current.append(chunk);
        return 0;
    }

    int end_elem(const std::string &elem){
        // name
        if(StrUtil::compare_ncase("name", elem) == 0) {
            property.filename = current.erase(0, prefix_to_remove.size());
        }

        // processing a Subdir? Add new dir
        if( StrUtil::compare_ncase("subdir", elem) == 0 && !property.filename.empty()){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new common prefix {}", current.c_str());
            property.info.mode =  0755 | S_IFDIR;
            property.info.mode &= ~(S_IFREG);
            props.push_back(property);
            property.clear();
        }

        // object has ended? push new entry
        if( StrUtil::compare_ncase("object", elem) == 0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new element {}", elem.c_str());
            property.info.mode = 0755;
            props.push_back(property);
            property.clear();
        }

        if( StrUtil::compare_ncase("bytes", elem) == 0){
            try{
                dav_size_t size = toType<dav_size_t, std::string>()(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element size {}", size);
                property.info.size = size;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element size");
            }
        }

        if( StrUtil::compare_ncase("last_modified", elem) == 0){
            try{
                time_t mtime = S3::s3TimeConverter(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element LastModified {}", current);
                property.info.mtime = mtime;
                property.info.ctime = mtime;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element LastModified");
            }
        }

        // listing containers? push new entry
        if( StrUtil::compare_ncase("container", elem) == 0 && !property.filename.empty()){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new element {}", elem.c_str());
            property.info.mode =  0755 | S_IFDIR;
            property.info.mode &= ~(S_IFREG);
            props.push_back(property);
        }

        return 0;
    }
};


SwiftPropParser::SwiftPropParser() : d_ptr(new Internal())
{
    SwiftPropParser("");
}

SwiftPropParser::SwiftPropParser(std::string prefix) : d_ptr(new Internal())
{
    if(!prefix.empty()){
        if(prefix[prefix.size() - 1] != '/') {
            d_ptr->prefix_to_remove = prefix.erase(0,1) + "/";
        }
        else {
            d_ptr->prefix_to_remove = prefix;
        }

        if(d_ptr->prefix_to_remove == "/") {
            d_ptr->prefix_to_remove = "";
        }
    }

}

SwiftPropParser::~SwiftPropParser(){ }


int SwiftPropParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    /*(void) parent;
    (void) nspace;
    (void) atts;
    return d_ptr->start_elem(std::string(name)); */

    return d_ptr->start_elem(std::string(name));
}

int SwiftPropParser::parserCdataCb(int state, const char *cdata, size_t len){
    /*(void) state;
    (void) len;
    return d_ptr->add_chunk(std::string(cdata, len));*/
    //std::cout << "cdata: " << std::string(cdata, len) << std::endl;
    return d_ptr->add_chunk(std::string(cdata, len));
}

int SwiftPropParser::parserEndElemCb(int state, const char *nspace, const char *name){
    /*(void) state;
    (void) nspace;
    return d_ptr->end_elem(std::string(name));*/

    return d_ptr->end_elem(std::string(name));
}

std::deque<FileProperties> & SwiftPropParser::getProperties(){
    return d_ptr->props;
}
}