#ifndef METALINKPARSER_HPP
#define METALINKPARSER_HPP

#include <file/davfile.hpp>
#include <xml/davxmlparser.hpp>



namespace Davix{


namespace MetalinkTag{
enum MetalinkParserTag{
    Metalink = 0x00,
    Files = 0x01,
    File = 0x02,
    Size = 0x03,
    Resources = 0x04,
    Url = 0x05,
    Invalid = 0xFF
};

}

typedef  std::vector<MetalinkTag::MetalinkParserTag>  MetalinkStack;


class MetalinkParser : public XMLSAXParser
{
public:
    struct MetalinkParserIntern;

    MetalinkParser(Context & c, std::vector<DavFile> & vec);
    virtual ~MetalinkParser();

    dav_size_t getSize() const;

protected:

    virtual int parserStartElemCb(int parent,
                                   const char *nspace, const char *name,
                                   const char **atts);
    virtual int parserCdataCb(int state,
                                const char *cdata, size_t len);
    virtual int parserEndElemCb(int state,
                                const char *nspace, const char *name);

private:
    MetalinkParserIntern* d_ptr;
};


}

#endif // METALINKPARSER_HPP
