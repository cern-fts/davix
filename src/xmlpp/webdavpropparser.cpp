#include "webdavpropparser.h"
#include "davixxmlparserexception.h"

#define _GNU_SOURCE

#include <glibmm/quark.h>
#include <glibmm/datetime.h>
#include <glibmm.h>


#include <datetime/datetime_utils.h>

namespace Davix {

inline bool match_element(const std::string & origin, const std::string &pattern){
    bool res = false;
    size_t pos = origin.rfind(":");
    if(pos != std::string::npos){
        res=(origin.compare(pos+1, origin.length()- pos-1, pattern)==0)?true:false;
    }
    return res;
}

/**
  check if the current element origin match the pattern
  if it is the case, force the scope_bool -> TRUE, if already to TRUE -> error
  in a case of a change, return true, else false
*/
inline bool add_scope(bool *scope_bool, const std::string & origin, const std::string &pattern){
    if(match_element(origin, pattern)){
        if(*scope_bool==true){
            throw DavixXmlParserException(Glib::Quark("WebdavPropParser::add_scope"), EINVAL,
                       Glib::ustring::compose(" parsing error in the webdav request result, <%1> duplicated ", origin));
        }
        //std::cout << " in scope " << origin << " while parsing " << std::endl;
        *scope_bool = true;
        return true;
    }
    return false;
}

/**
  act like add_scope but in the opposite way
*/
inline bool remove_scope(bool *scope_bool, const std::string & origin, const std::string &pattern){
    if(match_element(origin, pattern)){
        if(*scope_bool==false){
           throw DavixXmlParserException(Glib::Quark("WebdavPropParser::remove_scope"), EINVAL,
                       Glib::ustring::compose(" parsing error in the webdav request result, </%1> not open before ", origin));
        }
        //std::cout << " out scope " << origin << " while parsing " << std::endl;
        *scope_bool = false;
        return true;
    }
    return false;
}



WebdavPropParser::WebdavPropParser()
{
}
WebdavPropParser::~WebdavPropParser()
{
}

void WebdavPropParser::on_start_document(){
    davix_log_debug(" -> parse request for properties ");
    prop_section = propname_section = false;
    response_section = lastmod_section = false;
    creatdate_section = contentlength_section= false;
}

void WebdavPropParser::on_end_document(){
    davix_log_debug(" -> end of parse request for properties ");
}

void WebdavPropParser::on_start_element(const std::string &name, const AttributeList &attributes){
    //std::cout << " name : " << name << std::endl;
    // compute the current scope
    bool new_prop= add_scope(&prop_section, name, "prop");
    add_scope(&propname_section, name, "propstat");
    add_scope(&response_section, name, "response");
    add_scope(&lastmod_section, name, "getlastmodified");
    add_scope(&creatdate_section, name, "creationdate");
    add_scope(&contentlength_section, name, "getcontentlength");
    // collect information
    if(new_prop)
        compute_new_elem();
}

void WebdavPropParser::on_end_element(const std::string &name){
    //std::cout << "name : " << name << std::endl;
    // compute the current scope
    remove_scope(&prop_section, name, "prop");
    remove_scope(&propname_section, name, "propstat");
    remove_scope(&response_section, name, "response");
    remove_scope(&lastmod_section, name, "getlastmodified");
    remove_scope(&creatdate_section, name, "creationdate");
    remove_scope(&contentlength_section, name, "getcontentlength");
}

const std::vector<FileProperties> & WebdavPropParser::parser_properties_from_memory(const std::string &str){
    _props.clear();
    parse_memory(str);
    return _props;
}

void WebdavPropParser::compute_new_elem(){
    if(prop_section && propname_section && response_section){
        davix_log_debug(" properties detected ");
        _props.push_back(FileProperties());
    }
}

void WebdavPropParser::check_last_modified(const std::string & chars){
    if(response_section && prop_section && propname_section
          && lastmod_section){ // parse rfc1123 date format
        davix_log_debug(" getlastmodified found -> parse it ");
        GError * tmp_err;
        time_t t = parse_http_date(chars.c_str(), &tmp_err);
        if(t == -1){
            DavixXmlParserException ex(g_quark_to_string(tmp_err->domain), ECOMM, tmp_err->message);
            g_clear_error(&tmp_err);
            throw ex;
        }
        davix_log_debug(" getlastmodified found -> value %ld ", t);
        _props.back().mtime = t;
    }
}

void WebdavPropParser::check_creation_date(const std::string & chars){
    if(response_section && prop_section && propname_section
            && creatdate_section){
        davix_log_debug("creationdate found -> parse it");
        GError * tmp_err=NULL;
        time_t t = parse_iso8601date(chars.c_str(), &tmp_err);
        if(t == -1){
            DavixXmlParserException ex(g_quark_to_string(tmp_err->domain), ECOMM, tmp_err->message);
            g_clear_error(&tmp_err);
            throw ex;
        }
        davix_log_debug(" creationdate found -> value %ld ", t);
        _props.back().ctime = t;
    }
}

void WebdavPropParser::check_content_length(const std::string &chars){
    if(response_section && prop_section && propname_section
             && contentlength_section){
        davix_log_debug((" content length found -> parse it"));
        const unsigned long mysize = strtoul(chars.c_str(), NULL, 10);
        if(mysize == ULONG_MAX){
            errno =0;
            throw DavixXmlParserException(Glib::Quark("WebdavPropParser::check_content_length"), ECOMM, " Invalid content length value in dav response");
        }
        davix_log_debug(" content length found -> %ld", mysize);
        _props.back().size = (off_t) mysize;
    }
}

void WebdavPropParser::on_characters(const std::string &characters){
    //std::cout << "chars ..." << characters<< std::endl;
    check_last_modified(characters);
    check_creation_date(characters);
    check_content_length(characters);
}



} // namespace DAvix

