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
#include "davdeletexmlparser.hpp"

#include <utils/davix_logger_internal.hpp>
#include <status/davixstatusrequest.hpp>
#include "libs/datetime/datetime_utils.hpp"
#include <utils/stringutils.hpp>

using namespace StrUtil;

namespace Davix {


const Xml::XmlPTree prop_response(Xml::ElementStart, "response");

static std::unique_ptr<Xml::XmlPTree> webDavTree;

static std::once_flag _l_init;

struct DavDeleteXMLParser::DavxDeleteXmlIntern{
    DavxDeleteXmlIntern() : _stack(),
        _props(), _current_props(), _last_response_status(500), _last_filename(){
        _stack.reserve(10);
        char_buffer.reserve(1024);
    }

    // node stack
    std::vector<Xml::XmlPTree> _stack;

    // props
    std::deque<FileProperties> _props;
    FileProperties _current_props;
    int _last_response_status;
    std::string _last_filename;

    // buffer
    std::string char_buffer;

    inline void appendChars(const char *buff, size_t len){
        char_buffer.append(std::string(buff, len));
    }

    inline void clear(){
        char_buffer.clear();
    }


    inline void add_new_elem(){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " properties detected ");
        _current_props.clear();
        _current_props.filename = _last_filename; // setup the current filename
        _current_props.req_status = _last_response_status;
    }

    inline void store_new_elem(){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " end of properties... ");
        _props.push_back(_current_props);
    }

    inline void update_elem(){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " updating propertie's info ");
    _current_props.filename = _last_filename;
    _current_props.req_status = _last_response_status;
}

};

typedef void (*properties_cb)(DavDeleteXMLParser::DavxDeleteXmlIntern & par, const std::string & name);

static void check_href(DavDeleteXMLParser::DavxDeleteXmlIntern & par,  const std::string & name){
    std::string _href(name);
    rtrim(_href, isSlash()); // remove trailing slash
    std::string::reverse_iterator it = std::find(_href.rbegin(), _href.rend(), '/');
    if( it == _href.rend()){
        par._last_filename.assign(_href);
    }else{
        par._last_filename.assign(it.base(), _href.end());
    }
   DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " href/filename parsed -> {} ", par._last_filename.c_str() );
}

static void check_status(DavDeleteXMLParser::DavxDeleteXmlIntern & par, const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " status found -> parse it");
    std::string str_status(name);
    ltrim(str_status, StrUtil::isSpace());
    std::string::iterator it1, it2;
    it1 = std::find(str_status.begin(), str_status.end(), ' ');
    if( it1 != str_status.end()){
        it2 = std::find(it1+1, str_status.end(), ' ');
        std::string str_status_parsed(it1+1, it2);
        unsigned long res = strtoul(str_status_parsed.c_str(), NULL, 10);
        if(res != ULONG_MAX){
           DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " status value : {}", res);
           par._last_response_status = res;
           return;
        }
    }
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, "Invalid dav status field value");
    errno =0;
}

static void init_webdavTree(){
    // Nodes list
    webDavTree.reset(new Xml::XmlPTree(Xml::ElementStart, "multistatus"));
    webDavTree->addChild(Xml::XmlPTree(Xml::ElementStart, "response"));
    Xml::XmlPTree::iterator it = webDavTree->beginChildren();
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "href", Xml::XmlPTree::ChildrenList(),  (void*) &check_href));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "status", Xml::XmlPTree::ChildrenList(), (void*) &check_status));
}

DavDeleteXMLParser::DavDeleteXMLParser() :
    d_ptr(new DavxDeleteXmlIntern())
{
    std::call_once(_l_init, init_webdavTree);
}

DavDeleteXMLParser::~DavDeleteXMLParser(){
    delete d_ptr;
}


std::deque<FileProperties> & DavDeleteXMLParser::getProperties(){
    return d_ptr->_props;
}


int DavDeleteXMLParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) name;
    (void) nspace;
    (void) atts;
    // add elem to stack
    Xml::XmlPTree node(Xml::ElementStart, name);
    d_ptr->_stack.push_back(node);

    // if beginning of prop, add new element
    if(node.compareNode(prop_response)){
        d_ptr->add_new_elem();
    }

    return 1;
}

int DavDeleteXMLParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    d_ptr->appendChars(cdata, len);
    return 0;
}


int DavDeleteXMLParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    Xml::XmlPTree node(Xml::ElementStart, name);

    if(node.compareNode(prop_response)){
        d_ptr->store_new_elem();
    }

    // find potential interesting data
    std::vector<Xml::XmlPTree::ptr_type> chain;
    if(d_ptr->char_buffer.size() != 0){
        chain = webDavTree->findChain(d_ptr->_stack);
        if(chain.size() > 0){
            properties_cb cb = ((properties_cb) chain.at(chain.size()-1)->getMeta());
            if(cb){
                StrUtil::trim(d_ptr->char_buffer);
                cb(*d_ptr, d_ptr->char_buffer);
            }
        }
        d_ptr->update_elem();
    }

    // cleaning work
    if(d_ptr->_stack.size()  == 0)
        throw DavixException(davix_scope_xml_parser(),StatusCode::ParsingError, "Corrupted Parser Stack, Invalid XML");
    d_ptr->_stack.pop_back();
    d_ptr->clear();
    return 0;
}

} // namespace Davix
