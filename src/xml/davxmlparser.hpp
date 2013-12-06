#ifndef DAVIX_DAVXMLPARSER_H
#define DAVIX_DAVXMLPARSER_H


#include <ne_xml.h>
#include <davix.hpp>

namespace Davix {

class XMLSAXParser : NonCopyable
{
public:
    XMLSAXParser();
    virtual ~XMLSAXParser();

    //
    // parse a block of character with a maximum size of 'len' characters
    // return negative value if failure or 0 if success
    int parseChuck(const char * partial_string, size_t len);


    // return last error, dnamically allocated, need to be free
    Davix::DavixError* getLastErr();

protected:
    Davix::DavixError * err;

    ///
    /// callback to reimplement in subclass for parsing
    /// codes :
    ///  retcode < 0  -> error
    ///  retcode == 0 -> skip this element
    ///  retcode > 0 -> accept this element

    /// start element callback
    virtual int parserStartElemCb(int parent,
                                   const char *nspace, const char *name,
                                   const char **atts);

   /// cdata element callback
   virtual int parserCdataCb(int state,
                                const char *cdata, size_t len);

    /// end element callback
    virtual int parserEndElemCb(int state,
                                const char *nspace, const char *name);

private:
    ne_xml_parser*  _ne_parser;
    friend struct InternalDavParser;
};

} // namespace Davix

#endif // DAVIX_DAVXMLPARSER_H
