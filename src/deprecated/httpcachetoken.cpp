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



namespace Davix {

struct HttpCacheTokenInternal{

    HttpCacheTokenInternal() :
        _req_uri(),
        _redirection_uri()
    {}

    HttpCacheTokenInternal(const HttpCacheTokenInternal & orig) :
        _req_uri(orig._req_uri),
        _redirection_uri(orig._redirection_uri)

    {

    }

    virtual ~HttpCacheTokenInternal(){}



    Uri _req_uri, _redirection_uri;
private:
    HttpCacheTokenInternal & operator=(const HttpCacheTokenInternal &);
};


HttpCacheToken::HttpCacheToken() :
    d_ptr(new HttpCacheTokenInternal())
{

}

HttpCacheToken::HttpCacheToken(const HttpCacheToken &orig) :
    d_ptr(new HttpCacheTokenInternal(*(orig.d_ptr)))
{

}

HttpCacheToken & HttpCacheToken::operator=(const HttpCacheToken & orig){
    if(&orig == this)
        return *this;
    delete d_ptr;
    d_ptr = new HttpCacheTokenInternal(*(orig.d_ptr));
    return *this;
}

HttpCacheToken::~HttpCacheToken(){
    delete d_ptr;
}


const Uri  & HttpCacheToken::getCachedRedirection() const{
    return d_ptr->_redirection_uri;
}

const Uri & HttpCacheToken::getrequestUri() const {
    return d_ptr->_req_uri;
}


// Deprecated replicas
// created for Abi compatibility
void deprecated_abi_calls(){
    Replica* r = new Replica();
    ReplicaVec* v = new ReplicaVec();
    v->resize(1);

    FileInfo<FileInfoProtocolType>* t = new FileInfo<FileInfoProtocolType>();
    delete (t->getClone());
    delete t;
    FileInfoSize* s = new FileInfoSize();
    delete s;
    delete r;
    delete v;
}



HookTraits::HookTraits() : d_ptr(NULL){

}

HookTraits::~HookTraits(){

}




}

