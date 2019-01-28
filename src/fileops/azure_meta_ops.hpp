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

#ifndef DAVMETA_HPP
#define DAVMETA_HPP

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davfile.hpp>
#include <fileops/httpiochain.hpp>

namespace Davix{

struct DirHandle;

///
/// \brief The HttpMetaOps class
///
/// The HttpMetaOps handle all meta-data operations related to HTTP/Webdav
class HttpMetaOps : public HttpIOChain{
public:
    HttpMetaOps();
    virtual ~HttpMetaOps();

    // calculate hecksum
    virtual void checksum(IOChainContext & iocontext, std::string & checksm, const std::string & chk_algo);

    // delete resource
    virtual void deleteResource(IOChainContext & iocontext);

    // make collection
    virtual void makeCollection(IOChainContext & iocontext);

    // move/rename resource
    virtual void move(IOChainContext & iocontext, const std::string & target_url);

    // get statInfo
    virtual StatInfo & statInfo(IOChainContext & iocontext, StatInfo & st_info);

    virtual bool nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info);

private:

    std::unique_ptr<DirHandle> directoryItem;
};


/// The HttpMetaOps handle all meta-data operations related to Aws S3
class S3MetaOps : public HttpIOChain{
public:
    S3MetaOps();
    virtual ~S3MetaOps();

    // S3 + HTTP checksum computation
    virtual void checksum(IOChainContext & iocontext, std::string & checksm, const std::string & chk_algo);

    // get statInfo
    virtual StatInfo & statInfo(IOChainContext & iocontext, StatInfo & st_info);

    // make collection
    virtual void makeCollection(IOChainContext & iocontext);

    // listing
    virtual bool nextSubItem(IOChainContext &iocontext, std::string &entry_name, StatInfo &info);

private:
    std::unique_ptr<DirHandle> directoryItem;

};



/*
  retrieve a webdav propfind stat request to the given url
    @param req : http request where to executethe query
    @return vector of characters of the query content
  */
std::vector<char> req_webdav_propfind(HttpRequest* req, DavixError** err);



} // Davix




#endif // DAVMETA_HPP
