#include <config.h>
#include "davpropxmlparser.hpp"
#include <logger/davix_logger_internal.h>
#include <status/davixstatusrequest.hpp>
#include <cstdlib>
#include <datetime/datetime_utils.hpp>
#include <string_utils/stringutils.hpp>
#include <lockers/dpponce.hpp>

namespace Davix {

const Xml::XmlPTree prop_node(Xml::ElementStart, "propstat");
const Xml::XmlPTree prop_collection(Xml::ElementStart, "collection");
static Xml::XmlPTree* webDavTree = NULL;
static DppOnce _l_init;

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
        DAVIX_DEBUG(" properties detected ");
        _current_props.clear();
        _current_props.filename = _last_filename; // setup the current filename
        _current_props.mode = 0777 | S_IFREG; // default : fake access to everything
    }

    inline void store_new_elem(){
        DAVIX_DEBUG(" end of properties... ");
        if( _last_response_status > 100
            && _last_response_status < 400){
            _props.push_back(_current_props);
        }else{
           DAVIX_DEBUG("Bad status code ! properties dropped");
        }
    }

};


typedef void (*properties_cb)(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name);


static void check_last_modified(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_DEBUG(" getlastmodified found -> parse it ");
    time_t t = parse_standard_date(name.c_str());
    if(t == -1){
        DAVIX_LOG(DAVIX_LOG_WARNING, " getlastmodified parsing error : corrupted value ... ignored");
        t = 0;
    }
    DAVIX_DEBUG(" getlastmodified found -> value %ld ", t);
    par._current_props.mtime = t;
}


static void check_creation_date(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_DEBUG("creationdate found -> parse it");
    time_t t = parse_standard_date(name.c_str());
    if(t == -1){
        DAVIX_LOG(DAVIX_LOG_WARNING," creationdate parsing error : corrupted value ... ignored");
        t = 0;
    }
    DAVIX_DEBUG(" creationdate found -> value %ld ", t);
    par._current_props.ctime = t;
}

static void check_is_directory(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
   DAVIX_DEBUG(" directory pattern found -> set flag IS_DIR");
   par._current_props.mode |=  S_IFDIR;
   par._current_props.mode &= ~(S_IFREG);
}


static void check_content_length(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    DAVIX_DEBUG(" content length found -> parse it");
    const unsigned long mysize = strtoul(name.c_str(), NULL, 10);
    if(mysize == ULONG_MAX){
        DAVIX_LOG(DAVIX_LOG_WARNING," Invalid content length value in dav response");
        errno =0;
        return;
    }
    DAVIX_DEBUG(" content length found -> %ld", mysize);
    par._current_props.size = (off_t) mysize;
}

static void check_mode_ext(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_DEBUG(" mode_t extension for LCGDM found -> parse it");
    const unsigned long mymode = strtoul(name.c_str(), NULL, 8);
    if(mymode == ULONG_MAX){
        DAVIX_LOG(DAVIX_LOG_WARNING," Invalid mode_t value for the LCGDM extension");
        errno =0;
        return;
    }
    DAVIX_DEBUG(" mode_t extension found -> 0%o", (mode_t) mymode);
    par._current_props.mode = (mode_t) mymode;
}

static void check_href(DavPropXMLParser::DavxPropXmlIntern & par,  const std::string & name){
    std::string _href(name);
    rtrim(_href, isslash); // remove trailing slash
    std::string::reverse_iterator it = std::find(_href.rbegin(), _href.rend(), '/');
    if( it == _href.rend()){
        par._last_filename.assign(_href);
    }else{
        par._last_filename.assign(it.base(), _href.end());
    }
   DAVIX_DEBUG(" href/filename parsed -> %s ", par._last_filename.c_str() );
}

static void check_status(DavPropXMLParser::DavxPropXmlIntern & par, const std::string & name){
    DAVIX_DEBUG(" status found -> parse it");
    std::string str_status(name);
    ltrim(str_status, static_cast<int (*)(int)>(std::isspace));
    std::string::iterator it1, it2;
    it1 = std::find(str_status.begin(), str_status.end(), ' ');
    if( it1 != str_status.end()){
        it2 = std::find(it1+1, str_status.end(), ' ');
        std::string str_status_parsed(it1+1, it2);
        unsigned long res = strtoul(str_status_parsed.c_str(), NULL, 10);
        if(res != ULONG_MAX){
           DAVIX_DEBUG(" status value : %ld", res);
           par._last_response_status = res;
           return;
        }
    }
    DAVIX_LOG(DAVIX_LOG_WARNING,"Invalid dav status field value");
    errno =0;
}


void init_webdavTree(){

    // Nodes list
    webDavTree = new Xml::XmlPTree(Xml::ElementStart, "multistatus");
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
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "getcontentlength", Xml::XmlPTree::ChildrenList(), (void*) &check_content_length));

    it->addChild(Xml::XmlPTree(Xml::ElementStart, "mode", Xml::XmlPTree::ChildrenList(), (void*) &check_mode_ext));
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "resourcetype"));
    it = (--it->endChildren());
    it->addChild(Xml::XmlPTree(Xml::ElementStart, "collection", Xml::XmlPTree::ChildrenList(), (void*) &check_is_directory));
}

DavPropXMLParser::DavPropXMLParser() :
    d_ptr(new DavxPropXmlIntern())
{
    _l_init.once(init_webdavTree);
}

DavPropXMLParser::~DavPropXMLParser(){
    delete d_ptr;
}


std::deque<FileProperties> & DavPropXMLParser::getProperties(){
    return d_ptr->_props;
}


int DavPropXMLParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
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
    d_ptr->appendChars(cdata, len);
    return 0;
}


int DavPropXMLParser::parserEndElemCb(int state, const char *nspace, const char *name){
    Xml::XmlPTree node(Xml::ElementStart, name);

    // find potential interesting data
    std::vector<Xml::XmlPTree::ptr_type> chain;
    if(d_ptr->char_buffer.size() != 0 || node.compareNode(prop_collection)){
        chain = webDavTree->findChain(d_ptr->_stack);
        if(chain.size() > 0){
            properties_cb cb = ((properties_cb) chain.at(chain.size()-1)->getMeta());
            if(cb)
                cb(*d_ptr, d_ptr->char_buffer);
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
