#include <config.h>
#include <logger/davix_logger_internal.h>
#include <status/davixstatusrequest.hpp>
#include <cstdlib>
#include <datetime/datetime_utils.h>
#include <string_utils/stringutils.hpp>
#include "davpropxmlparser.hpp"


const char * prop_pattern = "prop";
const char * propstat_pattern = "propstat" ;
const char * response_pattern = "response" ;
const char * getlastmodified_pattern = "getlastmodified" ;
const char * creationdate_pattern = "creationdate" ;
const char * getcontentlength_pattern = "getcontentlength" ;
const char * mode_pattern = "mode" ;
const char * href_pattern = "href" ;
const char * resource_type_patern = "resourcetype" ;
const char * collection_patern = "collection" ;
const char * status_pattern = "status" ;


const char* parser_elem_list_start[] = { prop_pattern };


#define DAVIX_XML_REPORT_ERROR(X) \
    do{                           \
        X;                        \
        if(err)                   \
            return -1;            \
    }while(0)


namespace Davix {


/**
  check if the current element origin match the pattern
  if it is the case, force the scope_bool -> TRUE, if already to TRUE -> error
  in a case of a change, return true, else false
*/
inline bool add_scope(bool *scope_bool, const char*  origin, const char* pattern,
                      const bool enter_condition,
                      const bool skip_condition, DavixError** err){
    if(enter_condition && !skip_condition && match_element(origin, pattern)){
        if(*scope_bool==true){
            DavixError::setupError(err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "parsing error in the webdav request result :" + std::string(origin) + "deplucated");
            return false;
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
inline bool remove_scope(bool *scope_bool, const char* origin, const char* pattern,
                        const bool enter_condition,
                        const bool skip_condition, DavixError** err){
    if(enter_condition && !skip_condition && match_element(origin, pattern)){
        if(*scope_bool==false){
            DavixError::setupError(err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "parsing error in the webdav request result :" + std::string(origin) + "not opended before");
            return false;
        }
        //std::cout << " out scope " << origin << " while parsing " << std::endl;
        *scope_bool = false;
        return true;
    }
    return false;
}


std::deque<FileProperties> _props;
FileProperties _current_props;

// scope boolean
bool prop_section;
bool propname_section;
bool response_section;
bool lastmod_section;
bool creatdate_section;
bool contentlength_section;
bool mode_ext_section;
bool href_section;
bool resource_type;
bool status_section;

std::string last_filename; // last filename section
std::string char_buffer;



DavPropXMLParser::DavPropXMLParser() :
    _props(),
    _current_props(),
    _last_response_status(404),
    prop_section(false),
    propname_section(false),
    response_section(false),
    lastmod_section(false),
    creatdate_section(false),
    contentlength_section(false),
    mode_ext_section(false),
    href_section(false),
    resource_type(false),
    status_section(false),
    last_filename(),
    char_buffer()
{
}

DavPropXMLParser::~DavPropXMLParser(){

}



int DavPropXMLParser::compute_new_elem(){
    if(prop_section && propname_section && response_section){
        DAVIX_DEBUG(" properties detected ");
        _current_props.clear();
        _current_props.filename = last_filename; // setup the current filename
        _current_props.mode = 0777 | S_IFREG; // default : fake access to everything
    }
    return 0;
}

int DavPropXMLParser::store_new_elem(){
    if(response_section){
        DAVIX_DEBUG(" end of properties... ");
        if( _last_response_status > 100
            && _last_response_status < 400){
            _props.push_back(_current_props);
        }else
           DAVIX_DEBUG(" Bad status code ! properties dropped ");
    }
    return 0;
}

int DavPropXMLParser::check_last_modified(const char* name){
    if(response_section && prop_section && propname_section
          && lastmod_section && char_buffer.empty() == false){ // parse rfc1123 date format
        DAVIX_DEBUG(" getlastmodified found -> parse it ");
        time_t t = parse_standard_date(name);
        if(t == -1){
            DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "Invalid last modified date format");
            return -1;
        }
        DAVIX_DEBUG(" getlastmodified found -> value %ld ", t);
        _current_props.mtime = t;
    }
    return 0;
}

int DavPropXMLParser::check_creation_date(const char* name){
    if(response_section && prop_section && propname_section
            && creatdate_section && char_buffer.empty() == false){
        DAVIX_DEBUG("creationdate found -> parse it");
        time_t t = parse_standard_date(name);
        if(t == -1){
            DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "Invalid creation date format");
           return -1;
        }
        DAVIX_DEBUG(" creationdate found -> value %ld ", t);
        _current_props.ctime = t;
    }
    return 0;
}

int DavPropXMLParser::check_is_directory(const char* name){
    if(response_section && prop_section && propname_section
            && resource_type){
        bool is_dir=false;
        add_scope(&is_dir, name, collection_patern, propname_section && resource_type, false, &err);
        if(err != NULL)
            return -1;
        if(is_dir){
           DAVIX_DEBUG(" directory pattern found -> set flag IS_DIR");
           _current_props.mode |=  S_IFDIR;
           _current_props.mode &= ~(S_IFREG);
        }
    }
    return 0;
}

int DavPropXMLParser::check_content_length(const char* name){
    if(response_section && prop_section && propname_section
             && contentlength_section && char_buffer.empty() == false){
        DAVIX_DEBUG(" content length found -> parse it");
        const unsigned long mysize = strtoul(name, NULL, 10);
        if(mysize == ULONG_MAX){
            DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, " Invalid content length value in dav response");
            errno =0;
            return -1;
        }
        DAVIX_DEBUG(" content length found -> %ld", mysize);
        _current_props.size = (off_t) mysize;
    }
    return 0;
}

int DavPropXMLParser::check_mode_ext(const char* name){
    if(response_section && prop_section && propname_section &&
            mode_ext_section && char_buffer.empty() == false){
        DAVIX_DEBUG(" mode_t extension for LCGDM found -> parse it");
        const unsigned long mymode = strtoul(name, NULL, 8);
        if(mymode == ULONG_MAX){
            DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, " Invalid mode_t value for the LCGDM extension");
            errno =0;
            return -1;
        }
        DAVIX_DEBUG(" mode_t extension found -> 0%o", (mode_t) mymode);
        _current_props.mode = (mode_t) mymode;
    }
    return 0;
}

int DavPropXMLParser::check_href(const char* c_name){
    if(response_section &&
            href_section && char_buffer.empty() == false){
        DAVIX_DEBUG(" href/filename found -> parse it");
        std::string _href(c_name);
        rtrim(_href, isslash); // remove trailing slash
        std::string::reverse_iterator it = std::find(_href.rbegin(), _href.rend(), '/');
        if( it == _href.rend()){
            std::swap(_href, last_filename);
        }else{
            last_filename.assign(it.base(), _href.end());
        }
       DAVIX_DEBUG(" href/filename parsed -> %s ", last_filename.c_str() );
    }
    return 0;
}

int DavPropXMLParser::check_status(const char* name){
    if(response_section &&
            propname_section && status_section ){
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
               _last_response_status = res;
               return 0;
            }
        }
        DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, " Invalid dav status field value");
        errno =0;
       return -1;
    }
    return 0;
}


int DavPropXMLParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    // compute the current scope
    bool new_prop, new_response;
    int ret=-1;
    char_buffer.clear();

    DAVIX_XML_REPORT_ERROR( add_scope(&propname_section, name, propstat_pattern,  response_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( new_prop = add_scope(&prop_section, name, prop_pattern, response_section && propname_section , false, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&status_section, name, status_pattern, propname_section && response_section, prop_section, &err) );
    DAVIX_XML_REPORT_ERROR( new_response= add_scope(&response_section, name, response_pattern, true, prop_section && propname_section, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&lastmod_section, name, getlastmodified_pattern, propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&creatdate_section, name, creationdate_pattern, propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&contentlength_section, name, getcontentlength_pattern,propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&mode_ext_section, name, mode_pattern,propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&href_section, name, href_pattern, response_section, prop_section, &err) );
    DAVIX_XML_REPORT_ERROR( add_scope(&resource_type, name, resource_type_patern, propname_section, false, &err) );
    // mono-balise check
    if( ( ret = check_is_directory(name)) < 0)
            return ret;
    // collect information
    if(new_response)
        _last_response_status = 404;
    if(new_prop)
        ret= compute_new_elem();
    return (ret==0)?1:-1;
}

int DavPropXMLParser::parserCdataCb(int state, const char *cdata, size_t len){
    appendChars(cdata, len);
    return 0;
}

int DavPropXMLParser::triggerCdataCbparsing(){
    const char * buff = char_buffer.c_str();
    DAVIX_XML_REPORT_ERROR( check_last_modified(buff) );
    DAVIX_XML_REPORT_ERROR( check_creation_date(buff) );
    DAVIX_XML_REPORT_ERROR( check_content_length(buff) );
    DAVIX_XML_REPORT_ERROR( check_mode_ext(buff) );
    DAVIX_XML_REPORT_ERROR( check_href(buff) );
    DAVIX_XML_REPORT_ERROR( check_status(buff) );
    char_buffer.clear();
    return 0;
}

int DavPropXMLParser::appendChars(const char *buff, size_t len){
    char_buffer.append(std::string(buff, len));
    return 0;
}

int DavPropXMLParser::parserEndElemCb(int state, const char *nspace, const char *name){
    // compute the current scope
    int ret =0;
    bool end_prop;
    DAVIX_XML_REPORT_ERROR(triggerCdataCbparsing());

    DAVIX_XML_REPORT_ERROR( end_prop=  remove_scope(&propname_section, name, propstat_pattern, response_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&prop_section, name, prop_pattern,  response_section && propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&status_section, name, status_pattern, propname_section && response_section, prop_section, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&response_section, name, response_pattern, true, prop_section && propname_section, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&lastmod_section, name, getlastmodified_pattern, propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&creatdate_section, name, creationdate_pattern, propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&contentlength_section, name, getcontentlength_pattern, propname_section, false, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&mode_ext_section, name, mode_pattern, propname_section, false, &err) ); // lcgdm extension for mode_t support
    DAVIX_XML_REPORT_ERROR( remove_scope(&href_section, name, href_pattern, response_section, prop_section, &err) );
    DAVIX_XML_REPORT_ERROR( remove_scope(&resource_type, name, resource_type_patern, propname_section, false, &err) );
    if(end_prop)
        ret = store_new_elem();
    return ret;
}

} // namespace Davix
