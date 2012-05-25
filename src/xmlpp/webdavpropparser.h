#ifndef DAVIX_WEBDAVPROPPARSER_H
#define DAVIX_WEBDAVPROPPARSER_H

#include <libxml++/parsers/saxparser.h>
#include <fileproperties.h>
#include <glibmm/ustring.h>
#include <cstring>

namespace Davix {

class WebdavPropParser : public xmlpp::SaxParser
{
public:
    WebdavPropParser();
    virtual ~WebdavPropParser();
    /**
      Parse a webdav request to a list of file properties
    */
    const std::vector<FileProperties> & parser_properties_from_memory(const Glib::ustring& name);
    /**
      parse a webdav request in chunk mode
    */
    const std::vector<FileProperties> & parser_properties_from_chunk(const Glib::ustring& str);

    /**
      return the current parsed properties, same that the last call to parser_properties_from_*
    */
    const std::vector<FileProperties> & get_current_properties();
    void clean();
private:
    bool prop_section;
    bool propname_section;
    bool response_section;
    bool lastmod_section;
    bool creatdate_section;
    bool contentlength_section;
    bool mode_ext_section;
    bool href_section;
    bool resource_type;

    Glib::ustring last_filename; // last filename section
    std::vector<FileProperties> _props;

    FileProperties _current_props;

     /**
      add a new properties to the properties queue if scope is ok
    */
    void compute_new_elem();
    void store_new_elem();

    void check_is_directory(const Glib::ustring & name);
    void check_last_modified(const Glib::ustring& name);
    void check_creation_date(const Glib::ustring& name);
    void check_content_length(const Glib::ustring& name);
    void check_mode_ext(const Glib::ustring& name);
    void check_href(const Glib::ustring & name);
protected:
    virtual void on_start_document();
    virtual void on_end_document();
    virtual void on_start_element(const Glib::ustring& name, const AttributeList& attributes);
    virtual void on_end_element(const Glib::ustring& name);
    virtual void on_characters(const Glib::ustring& name);



};

inline bool match_element(const Glib::ustring & origin, const Glib::ustring& pattern){ // C style optimized, critical function
    bool res = false;
    const char* c_origin =  origin.c_str();
    const char* c_pattern = pattern.c_str();
    const char* pos = strrchr(c_origin, ':');
    if(pos != NULL){
        res = (*(pos+1) == *(c_pattern) && strcmp(pos+1, c_pattern) ==0)?true:false;
    }
    return res;
}

} // namespace DAvix

#endif // DAVIX_WEBDAVPROPPARSER_H
