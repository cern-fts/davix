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

#include "azurepropparser.hpp"
#include <utils/davix_azure_utils.hpp>

#include <stack>
#include <utils/davix_logger_internal.hpp>


namespace Davix{

const std::string col_prop = "Name";
const std::string delimiter_prop ="Contents";
const std::string name_prop = "Key";
const std::string size_prop = "Size";

const std::string prefix_prop = "Prefix";
const std::string com_prefix_prop = "CommonPrefixes";
const std::string listbucketresult_prop = "ListBucketResult";
const std::string last_modified_prop = "LastModified";

/*struct S3PropParser::Internal{
    std::string current;
    std::string prefix;
    std::string prefix_to_remove;
    bool inside_com_prefix;
    int prop_count;
    std::stack<std::string> stack_status;
    std::deque<FileProperties> props;

    FileProperties property;
    S3ListingMode::S3ListingMode _s3_listing_mode;

    int start_elem(const std::string &elem){
        // new tag, clean content;
        current.clear();

        // check XML nested level, security protection
        if(stack_status.size() < 200){
            stack_status.push(elem);
        }else{
            throw DavixException(davix_scope_xml_parser(), StatusCode::ParsingError, "Impossible to parse S3 content, corrupted XML");
        }

        // check element, if collection name add first entry
        if( StrUtil::compare_ncase(col_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "collection found", elem.c_str());
            property.clear();
            prop_count = 0;
        }

        // check element, if new entry clear current entry
        if( StrUtil::compare_ncase(delimiter_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "new element found", elem.c_str());
            property.clear();
        }

        // check element, if common prefixes set flag
        if( (_s3_listing_mode == S3ListingMode::Hierarchical) && StrUtil::compare_ncase(com_prefix_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "common prefixes found", elem.c_str());
            inside_com_prefix = true;
        }

        // check element, if prefix clear current entry
        if( (_s3_listing_mode == S3ListingMode::Hierarchical) && StrUtil::compare_ncase(prefix_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "prefix found", elem.c_str());
            property.clear();
        }

        return 1;
    }

    int add_chunk(const std::string & chunk){
        current.append(chunk);
        return 0;
    }

    int end_elem(const std::string &elem){
        StrUtil::trim(current);

        // found prefix
        if( (_s3_listing_mode == S3ListingMode::Hierarchical) &&
                StrUtil::compare_ncase(prefix_prop, elem) ==0 &&
                !current.empty()){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "new prefix {}", current.c_str());
            prefix = current;
            if(inside_com_prefix){  // all keys would have been processed by now, just common prefixes left, use as DIRs
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new common prefix {}", current.c_str());
                current = current.erase(current.size()-1,1);
                property.filename = current.erase(0, prefix_to_remove.size());
                property.info.mode =  0755 | S_IFDIR;
                property.info.mode &= ~(S_IFREG);
                props.push_back(property);
                prop_count++;
            }
        }

        // new name new fileprop
        if( StrUtil::compare_ncase(name_prop, elem) ==0){

            if((_s3_listing_mode == S3ListingMode::Flat)){  // flat mode
                property.filename = current.erase(0,prefix.size());
            }
            else if(prefix.empty()){    // at root level
                property.filename = current;
            }
            else if(!prefix.empty()){
                if(prefix.compare((prefix.size()-1),1,"/")){ // prefix doesn't end with '/', file
                    property.filename = current;
                }
                else if(!(StrUtil::compare_ncase(prefix, current) ==0)){ // folder
                    property.filename = current.erase(0, prefix_to_remove.size());

                }
            }

            if(!property.filename.empty())
                property.info.mode = 0755;
        }

        if( StrUtil::compare_ncase(size_prop, elem) ==0){
            try{
                dav_size_t size = toType<dav_size_t, std::string>()(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element size {}", size);
                property.info.size = size;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element size");
            }
        }

        if( StrUtil::compare_ncase(last_modified_prop, elem) ==0){
            try{
                time_t mtime = S3::s3TimeConverter(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element LastModified {}", current);
                property.info.mtime = mtime;
                property.info.ctime = mtime;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element LastModified");
            }
        }

        // found bucket name
        // push it as first item to identify bucket
        if( StrUtil::compare_ncase(col_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push collection", elem.c_str());
            property.filename = current;
            property.info.mode |= S_IFDIR;
            property.info.mode &= ~(S_IFREG);
            props.push_back(property);
        }

        // check element, if end entry push new entry
        if( StrUtil::compare_ncase(delimiter_prop, elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new element {}", elem.c_str());
            props.push_back(property);
            prop_count++;
        }

        // check element, if end common prefix reset flag
        if( (_s3_listing_mode == S3ListingMode::Hierarchical) && StrUtil::compare_ncase(com_prefix_prop, elem) ==0){
            inside_com_prefix = false;
        }

        // end of xml respond and still no property, requested key exists but isn't a directory
        if( (_s3_listing_mode == S3ListingMode::Hierarchical) && (StrUtil::compare_ncase(listbucketresult_prop, elem) ==0) && (prop_count ==0) ){
            throw DavixException(davix_scope_directory_listing_str(), StatusCode::IsNotADirectory, "Not a S3 directory");
        }

        // reduce stack size
        if(stack_status.size() > 0)
            stack_status.pop();
        current.clear();
        return 0;
    }

    }; */

struct AzurePropParser::Internal {
    std::string current;
    std::string name;
    std::string prefix_to_remove;
    std::deque<FileProperties> props;
    FileProperties property;
    bool inside_prefix;

    int start_elem(const std::string &elem){
        // new tag, clean content;
        current.clear();

        // starting a new blob?
        if(StrUtil::compare_ncase("Blob", elem) ==0){
            inside_prefix = false;
        }

        // starting a blob prefix?
        if(StrUtil::compare_ncase("BlobPrefix", elem) == 0) {
            inside_prefix = true;
        }

        //std::cout << "start_elem: " << elem << std::endl;
        return 1;
    }

    int add_chunk(const std::string & chunk){
        current.append(chunk);
        return 0;
    }

    int end_elem(const std::string &elem){
        // name
        if(StrUtil::compare_ncase("Name", elem) == 0) {
            property.filename = current.erase(0, prefix_to_remove.size());
        }

        // processing a BlobPrefix? Add new dir
        if( StrUtil::compare_ncase("BlobPrefix", elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new common prefix {}", current.c_str());
            property.filename = current.erase(current.size()-1, 1);
            property.info.mode =  0755 | S_IFDIR;
            property.info.mode &= ~(S_IFREG);
            props.push_back(property);
        }

        // blob has ended? push new entry
        if( StrUtil::compare_ncase("Blob", elem) ==0){
            DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "push new element {}", elem.c_str());
            property.info.mode = 0755;
            props.push_back(property);
        }

        if( StrUtil::compare_ncase("Content-Length", elem) ==0){
            try{
                dav_size_t size = toType<dav_size_t, std::string>()(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element size {}", size);
                property.info.size = size;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element size");
            }
        }

        if( StrUtil::compare_ncase("Last-Modified", elem) ==0){
            try{
                time_t mtime = S3::s3TimeConverter(current);
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "element LastModified {}", current);
                property.info.mtime = mtime;
                property.info.ctime = mtime;
            }catch(...){
                DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_XML, "Unable to parse element LastModified");
            }
        }

        return 0;
    }
};


AzurePropParser::AzurePropParser(std::string prefix) : d_ptr(new Internal())
{
    if(prefix[prefix.size() - 1] != '/') {
        d_ptr->prefix_to_remove = prefix + "/";
    }
    else {
        d_ptr->prefix_to_remove = prefix;
    }

    if(d_ptr->prefix_to_remove == "/") {
        d_ptr->prefix_to_remove = "";
    }
}

AzurePropParser::~AzurePropParser(){ }


int AzurePropParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    /*(void) parent;
    (void) nspace;
    (void) atts;
    return d_ptr->start_elem(std::string(name)); */

    return d_ptr->start_elem(std::string(name));
}

int AzurePropParser::parserCdataCb(int state, const char *cdata, size_t len){
    /*(void) state;
    (void) len;
    return d_ptr->add_chunk(std::string(cdata, len));*/
    //std::cout << "cdata: " << std::string(cdata, len) << std::endl;
    return d_ptr->add_chunk(std::string(cdata, len));
}

int AzurePropParser::parserEndElemCb(int state, const char *nspace, const char *name){
    /*(void) state;
    (void) nspace;
    return d_ptr->end_elem(std::string(name));*/

    return d_ptr->end_elem(std::string(name));
}

std::deque<FileProperties> & AzurePropParser::getProperties(){
    return d_ptr->props;
}


}
