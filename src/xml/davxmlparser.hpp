#ifndef DAVIX_DAVXMLPARSER_H
#define DAVIX_DAVXMLPARSER_H


#include <utils/davix_types.hpp>
#include <status/davixstatusrequest.hpp>
#include <xml/davix_ptree.hpp>
#include <ne_xml.h>


namespace Davix {


class XMLSAXParser : NonCopyable
{
public:
    typedef std::string::iterator startIterChunk;
    typedef std::string::iterator endIterChunk;
    typedef std::pair<startIterChunk, endIterChunk> Chunk;


    XMLSAXParser();
    virtual ~XMLSAXParser();

    //
    // parse a block of character with a maximum size of 'len' characters
    // return negative value if failure or 0 if success
    int parseChuck(const char * partial_string, dav_size_t len);

protected:

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

    /// start element callback
    virtual int startElemCb(const Chunk & data,
                                   const std::vector<Chunk> & attrs);

    virtual int cdataCb(const Chunk & data);

    virtual int endElemCb(const Chunk & data);

    virtual int commentCb(const Chunk & data);

private:
    ne_xml_parser*  _ne_parser;
    friend struct InternalDavParser;
};

} // namespace Davix

#endif // DAVIX_DAVXMLPARSER_H
