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
#include "davpropxmlparser.hpp"

#include <utils/davix_logger_internal.hpp>
#include <status/davixstatusrequest.hpp>
#include "libs/datetime/datetime_utils.hpp"
#include <utils/stringutils.hpp>

using namespace StrUtil;

namespace Davix {

const Xml::XmlPTree prop_node(Xml::ElementStart, "propstat");
const Xml::XmlPTree prop_collection(Xml::ElementStart, "collection");
static std::unique_ptr<Xml::XmlPTree> webDavTree;

static std::once_flag _l_init;

struct DavPropXMLParser::DavxPropXmlIntern{
    DavxPropXmlIntern() : _stack(),
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
        _current_props.info.mode = 0777 | S_IFREG; // default : fake access to everything
    }

    inline void store_new_elem(){
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " end of properties... ");
        if( _last_response_status > 100
            && _last_response_status < 400){
            _props.push_back(_current_props);
        }else{
           DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, "Bad status code ! properties dropped");
        }
    }

};


typedef void (*properties_cb)(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name);


static void check_last_modified(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " getlastmodified found -> parse it ");
    time_t t = parse_standard_date(name.c_str());
    if(t == -1){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, " getlastmodified parsing error : corrupted value ... ignored");
        t = 0;
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " getlastmodified found -> value {} ", t);
    par._current_props.info.mtime = t;
}


static void check_creation_date(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, "creationdate found -> parse it");
    time_t t = parse_standard_date(name.c_str());
    if(t == -1){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, " creationdate parsing error : corrupted value ... ignored");
        t = 0;
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " creationdate found -> value {} ", t);
    par._current_props.info.ctime = t;
}

static void check_is_directory(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
   (void) name;
   DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " directory pattern found -> set flag IS_DIR");
   par._current_props.info.mode |=  S_IFDIR;
   par._current_props.info.mode &= ~(S_IFREG);
}


static void check_content_length(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " content length found -> parse it");
    try{
        const unsigned long mysize = toType<unsigned long, std::string>()(name);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " content length found -> {}", mysize);
        par._current_props.info.size = static_cast<off_t>(mysize);
    }catch(...){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, " Invalid content length value in dav response");
    }
}

static void check_quota_used_bytes(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " quota used bytes found -> parse it");
    try{
        const unsigned long mysize = toType<unsigned long, std::string>()(name);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " quota used bytes found -> {}", mysize);
        par._current_props.info.size = static_cast<off_t>(mysize);
        par._current_props.quota.used_bytes = static_cast<off_t>(mysize);
    }catch(...){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, " Invalid quota used bytes in dav response");
    }
}

static void check_quota_free_space(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " quota free space found -> parse it");
    try{
        const unsigned long mysize = toType<unsigned long, std::string>()(name);
        DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " quota free space found -> {}", mysize);
        par._current_props.quota.free_space = static_cast<off_t>(mysize);
    }catch(...){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, " Invalid quota free space in dav response");
    }
}

static void check_mode_ext(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, "mode_t extension for LCGDM found -> parse it");
    const unsigned long mymode = strtoul(name.c_str(), NULL, 8);
    if(mymode == ULONG_MAX){
        DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, "Invalid mode_t value for the LCGDM extension");
        errno =0;
        return;
    }
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, fmt::sprintf(" mode_t extension found -> 0%o", (mode_t) mymode).c_str());
    par._current_props.info.mode = (mode_t) mymode;
}

static bool startswith(const std::string &str, const std::string &prefix) {
  if(prefix.size() > str.size()) return false;

  for(size_t i = 0; i < prefix.size(); i++) {
    if(str[i] != prefix[i]) return false;
  }
  return true;
}

static void check_href(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    std::string _href(name);
    rtrim(_href, isSlash()); // remove trailing slash
    std::string::reverse_iterator it = std::find(_href.rbegin(), _href.rend(), '/');
    if( it == _href.rend()){
        par._last_filename.assign(_href);
    }else{
        par._last_filename.assign(it.base(), _href.end());

        if(startswith(name, "https://") || startswith(name, "http://") || startswith(name, "://") || startswith(name, "dav://") || startswith(name, "davs://")) {
          par._last_filename = Uri::unescapeString(par._last_filename);
        }
    }
   DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " href/filename parsed -> {} ", par._last_filename.c_str() );
}

static void check_status(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
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

static void check_owner_uid(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & value){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " owner found -> parse it");
    std::string str_owner(value);
    ltrim(str_owner, StrUtil::isSpace());
    unsigned long res = strtoul(str_owner.c_str(), NULL, 10);
    if(res != ULONG_MAX){
       DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " owner value : {}", res);
       par._current_props.info.owner = res;
       return;
    }
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, "Invalid owner field value");
}

static void check_group_gid(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & value){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " group found -> parse it");
    std::string str_group(value);
    ltrim(str_group, StrUtil::isSpace());
    unsigned long res = strtoul(str_group.c_str(), NULL, 10);
    if(res != ULONG_MAX){
       DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_XML, " group value : {}", res);
       par._current_props.info.group = res;
       return;
    }
    DAVIX_SLOG(DAVIX_LOG_VERBOSE, DAVIX_LOG_XML, "Invalid group field value");
}

void init_webdavTree(){

    // Nodes list
    webDavTree.reset(new Xml::XmlPTree(Xml::ElementStart, "multistatus"));
    webDavTree->addChild(Xml::XmlPTree(Xml::ElementStart, "response"));
    Xml::XmlPTree::iterator it = webDavTree->beginChildren();
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "href", Xml::XmlPTree::ChildrenList(),  (void*) &check_href));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "propstat"));
    it = (--it->endChildren());
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "status", Xml::XmlPTree::ChildrenList(), (void*) &check_status));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "prop"));
    it = (--it->endChildren());

    it->addChild(Xml::XmlPTree(Xml::ElementStart, "getlastmodified", Xml::XmlPTree::ChildrenList(),  (void*) &check_last_modified));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "creationdate", Xml::XmlPTree::ChildrenList(), (void*) &check_creation_date));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "quota-used-bytes", Xml::XmlPTree::ChildrenList(), (void*) &check_quota_used_bytes));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "quota-available-bytes", Xml::XmlPTree::ChildrenList(), (void*) &check_quota_free_space));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "getcontentlength", Xml::XmlPTree::ChildrenList(), (void*) &check_content_length));

    it->addChild(Xml::XmlPTree(Xml::ElementStart, "owner", Xml::XmlPTree::ChildrenList(), (void*) &check_owner_uid));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "group", Xml::XmlPTree::ChildrenList(), (void*) &check_group_gid));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "mode", Xml::XmlPTree::ChildrenList(), (void*) &check_mode_ext));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "resourcetype"));

    it = (--it->endChildren());
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "collection", Xml::XmlPTree::ChildrenList(), (void*) &check_is_directory));
}

DavPropXMLParser::DavPropXMLParser() :
    d_ptr(new DavxPropXmlIntern())
{
    std::call_once(_l_init, init_webdavTree);
}

DavPropXMLParser::~DavPropXMLParser(){
    delete d_ptr;
}


std::deque<FileProperties> & DavPropXMLParser::getProperties(){
    return d_ptr->_props;
}


int DavPropXMLParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) name;
    (void) nspace;
    (void) atts;
    // add elem to stack
    Xml::XmlPTree node(Xml::ElementStart, name);
    d_ptr->_stack.push_back(node);

    // if beginning of prop, add new element
    if(node.compareNode(prop_node)){
        d_ptr->add_new_elem();
    }
    return 1;
}


int DavPropXMLParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    d_ptr->appendChars(cdata, len);
    return 0;
}


int DavPropXMLParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    Xml::XmlPTree node(Xml::ElementStart, name);

    // find potential interesting data
    std::vector<Xml::XmlPTree::ptr_type> chain;
    if(d_ptr->char_buffer.size() != 0 || node.compareNode(prop_collection)){
        chain = webDavTree->findChain(d_ptr->_stack);
        if(chain.size() > 0){
            properties_cb cb = ((properties_cb) chain.at(chain.size()-1)->getMeta());
            if(cb){
                StrUtil::trim(d_ptr->char_buffer);
                cb(*d_ptr, d_ptr->char_buffer);
            }
        }
    }

    // push props
    if(node.compareNode(prop_node)){
        d_ptr->store_new_elem();
    }

    // cleaning work
    if(d_ptr->_stack.size()  == 0)
        throw DavixException(davix_scope_xml_parser(),StatusCode::ParsingError, "Corrupted Parser Stack, Invalid XML");
    d_ptr->_stack.pop_back();
    d_ptr->clear();
    return 0;
}

} // namespace Davix
