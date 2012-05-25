#ifndef DAVIX_WEBDAVPROPPARSER_H
#define DAVIX_WEBDAVPROPPARSER_H

#include <libxml++/parsers/saxparser.h>
#include <fileproperties.h>
#include <glibmm/ustring.h>

namespace Davix {

class WebdavPropParser : public xmlpp::SaxParser
{
public:
    WebdavPropParser();
    virtual ~WebdavPropParser();
    const std::vector<FileProperties> & parser_properties_from_memory(const Glib::ustring& name);

private:
    bool prop_section;
    bool propname_section;
    bool response_section;
    bool lastmod_section;
    bool creatdate_section;
    bool contentlength_section;
    bool mode_ext_section;

    std::vector<FileProperties> _props;

    FileProperties _current_props;

     /**
      add a new properties to the properties queue if scope is ok
    */
    void compute_new_elem();
    void store_new_elem();

    void check_last_modified(const Glib::ustring& name);
    void check_creation_date(const Glib::ustring& name);
    void check_content_length(const Glib::ustring& name);
    void check_mode_ext(const Glib::ustring& name);
protected:
    virtual void on_start_document();
    virtual void on_end_document();
    virtual void on_start_element(const Glib::ustring& name, const AttributeList& attributes);
    virtual void on_end_element(const Glib::ustring& name);
    virtual void on_characters(const Glib::ustring& name);



};



} // namespace DAvix

#endif // DAVIX_WEBDAVPROPPARSER_H
