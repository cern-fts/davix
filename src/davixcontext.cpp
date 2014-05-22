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
#include <davixcontext.hpp>

#include <utils/davix_uri.hpp>
#include <neon/neonsessionfactory.hpp>
#include <davix_context_internal.hpp>



static const std::string _version = DAVIX_VERSION "-" DAVIX_VERSION_TAG;


namespace Davix{


///  Implementation f the core logic in davix
struct ContextInternal
{
    ContextInternal(NEONSessionFactory * fsess):
        _fsess(fsess),
        _s_buff(65536),
        _timeout(300),
        _context_flags(0),
        _hooks()
    {
        std::fill(_hooks, _hooks+ DAVIX_HOOK_REQUEST_NUM*2, static_cast<void*>(NULL));
    }

    ContextInternal(const ContextInternal & orig):
        _fsess(new NEONSessionFactory()),
        _s_buff(orig._s_buff),
        _timeout(orig._timeout),
        _context_flags(orig._context_flags),
        _hooks()
    {
        std::copy(orig._hooks, orig._hooks+ DAVIX_HOOK_REQUEST_NUM*2, _hooks);
    }

    virtual ~ContextInternal(){}

    // implementation of getSessionFactory
    inline NEONSessionFactory* getSessionFactory(){
         return _fsess.get();
    }

    void setBufferSize(const dav_size_t value){
      _s_buff = value;
    }

    boost::scoped_ptr<NEONSessionFactory>  _fsess;
    dav_size_t _s_buff;
    unsigned long _timeout;
    bool _context_flags;
    void* _hooks[DAVIX_HOOK_REQUEST_NUM*2];
};

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


Context::Context() :
    _intern(new ContextInternal(new NEONSessionFactory()))
{
}

Context::Context(const Context &c) :
    _intern(new ContextInternal(*(c._intern))){
}

Context & Context::operator=(const Context & c){
    if( this != &c ){
        if( _intern != NULL)
            delete _intern;
        _intern = new ContextInternal(*(c._intern));
    }
    return *this;
}

Context::~Context(){
    delete _intern;
}

Context* Context::clone(){
    return new Context(*this);
}


void Context::setSessionCaching(bool caching){
    _intern->_fsess->setSessionCaching(caching);
}

bool Context::getSessionCaching() const{
    return _intern->_fsess->getSessionCaching();
}


HttpRequest* Context::createRequest(const std::string & url, DavixError** err){
    return new HttpRequest(*this, Uri(url), err);
}


HttpRequest* Context::createRequest(const Uri &uri, DavixError **err){
    return new HttpRequest(*this, uri, err);
}


void Context::setHookById(int id, void* hook, void* userdata){
    if(id >=0 && id < DAVIX_HOOK_REQUEST_NUM-1)
        _intern->_hooks[2*id] = hook;
        _intern->_hooks[2*id+1] = userdata;
}

std::pair<void*,void*> Context::getHookById(int id){
    if(id >=0 && id < DAVIX_HOOK_REQUEST_NUM-1){
        return std::pair<void*,void*>(_intern->_hooks[2*id], _intern->_hooks[2*id+1]);
    }
    return std::pair<void*,void*>(static_cast<void*>(NULL),static_cast<void*>(NULL));
}

NEONSessionFactory & ContextExplorer::SessionFactoryFromContext(Context & c){
    return *static_cast<NEONSessionFactory*>(c._intern->getSessionFactory());
}


const std::string & version(){
    return _version;
}


} // End Davix

