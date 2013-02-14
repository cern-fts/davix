#ifndef DAVIX_DAVPROPXMLPARSER_HPP
#define DAVIX_DAVPROPXMLPARSER_HPP


#include <deque>
#include <xml/davxmlparser.hpp>
#include <fileproperties.hpp>
#include <string.h>

namespace Davix {

class DavPropXMLParser : public DavXMLParser
{
public:
    DavPropXMLParser();
    virtual ~DavPropXMLParser();

    std::deque<FileProperties> & getProperties(){
        return _props;
    }


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);

    // executed when cdata is complete
    int triggerCdataCbparsing();
    // append current chars to the internal buffer
    int appendChars(const char * buff, size_t len);
private:
    // result store
    std::deque<FileProperties> _props;
    FileProperties _current_props;
    int _last_response_status;

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

    // ordered list of properties

    //
    int compute_new_elem();
    int store_new_elem();

    int check_is_directory(const char* name);
    int check_last_modified(const char* name);
    int check_creation_date(const char* name);
    int check_content_length(const char* name);
    int check_mode_ext(const char* name);
    int check_href(const char* name);
    int check_status(const char* name);
};


inline bool match_element(const char* c_origin, const char* c_pattern){ // C style, critical function
    return strcmp(c_origin,c_pattern) ==0;
}

} // namespace Davix

#endif // DAVIX_DAVPROPXMLPARSER_HPP
