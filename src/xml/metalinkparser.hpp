#ifndef METALINKPARSER_HPP
#define METALINKPARSER_HPP

#include <xml/davxmlparser.hpp>
#include <davix_file_types.hpp>
#include <vector>
#include <stack>


namespace Davix{


namespace MetalinkTag{
enum MetalinkParserTag{
    Metalink3 = 0x00,
    Files = 0x01,
    File = 0x02,
    Size = 0x03,
    Resources = 0x04,
    Url = 0x05,
    Invalid = 0xFF
};

}

typedef  std::vector<MetalinkTag::MetalinkParserTag>  MetalinkStack;


class MetalinkParser : public XMLSAXParser, NonCopyable
{
public:
    MetalinkParser();
    virtual ~MetalinkParser();

    const ReplicaVec & getReplicas();

    const Properties & getProps();


protected:

    virtual int parserStartElemCb(int parent,
                                   const char *nspace, const char *name,
                                   const char **atts);
    virtual int parserCdataCb(int state,
                                const char *cdata, size_t len);
    virtual int parserEndElemCb(int state,
                                const char *nspace, const char *name);

private:
    ReplicaVec* rep;
    Properties* fileProperties;
    MetalinkStack tagStack;
    MetalinkParser(const MetalinkParser &);
    MetalinkParser & operator =(const MetalinkParser &);
};


}

#endif // METALINKPARSER_HPP
