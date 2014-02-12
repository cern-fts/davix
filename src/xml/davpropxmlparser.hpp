#ifndef DAVIX_DAVPROPXMLPARSER_HPP
#define DAVIX_DAVPROPXMLPARSER_HPP


#include <deque>
#include <xml/davxmlparser.hpp>
#include <fileproperties.hpp>
#include <string.h>

namespace Davix {


class DavPropXMLParser : public XMLSAXParser
{
public:
    struct DavxPropXmlIntern;
    DavPropXMLParser();
    virtual ~DavPropXMLParser();

    std::deque<FileProperties> & getProperties();


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);


private:
    DavxPropXmlIntern* d_ptr;
};


inline bool match_element(const char* c_origin, const char* c_pattern){ // C style, critical function
    return strcmp(c_origin,c_pattern) ==0;
}

} // namespace Davix

#endif // DAVIX_DAVPROPXMLPARSER_HPP
