#ifndef DAVIX_WEBDAVPROPPARSER_H
#define DAVIX_WEBDAVPROPPARSER_H

#include <libxml++/parsers/saxparser.h>
#include <fileproperties.h>

namespace Davix {

class WebdavPropParser : public xmlpp::SaxParser
{
public:
    WebdavPropParser();
    virtual ~WebdavPropParser();
    const std::vector<FileProperties> & parser_properties_from_memory(const std::string & str);

private:
    bool prop_section;
    bool propname_section;
    bool response_section;
    bool lastmod_section;
    bool creatdate_section;
    bool contentlength_section;

    std::vector<FileProperties> _props;

     /**
      add a new properties to the properties queue if scope is ok
    */
    virtual void compute_new_elem();

    void check_last_modified(const std::string & chars);
    void check_creation_date(const std::string & chars);
    void check_content_length(const std::string & chars);
protected:
    virtual void on_start_document();
    virtual void on_end_document();
    virtual void on_start_element(const std::string &name, const AttributeList &attributes);
    virtual void on_end_element(const std::string &name);
    virtual void on_characters(const std::string &characters);



};



} // namespace DAvix

#endif // DAVIX_WEBDAVPROPPARSER_H
