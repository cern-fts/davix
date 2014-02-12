#include "davxmlparser.hpp"

namespace Davix {

struct InternalDavParser{
    static int dav_xml_parser_ne_xml_startelm_cb(void *userdata, int parent,
                                   const char *nspace, const char *name,
                                   const char **atts){
        return static_cast<XMLSAXParser*>(userdata)->parserStartElemCb(parent, nspace, name, atts);
    }

    static int dav_xml_ne_xml_cdata_cb(void *userdata, int state,
                                const char *cdata, size_t len){
        return static_cast<XMLSAXParser*>(userdata)->parserCdataCb(state, cdata, len);
    }

    static int ne_xml_endelm_cb(void *userdata, int state,
                                 const char *nspace, const char *name){
        return static_cast<XMLSAXParser*>(userdata)->parserEndElemCb(state,nspace, name);
    }

};

int davParserNotImplemented(){
    throw Davix::DavixException(davix_scope_xml_parser(), StatusCode::OperationNonSupported, "the parser callbacks are not configured properly");
    return -1;
}

XMLSAXParser::XMLSAXParser() :  _ne_parser(ne_xml_create())
{
    ne_xml_push_handler(_ne_parser,
                        &InternalDavParser::dav_xml_parser_ne_xml_startelm_cb,
                        &InternalDavParser::dav_xml_ne_xml_cdata_cb,
                        &InternalDavParser::ne_xml_endelm_cb,
                        this);
}

XMLSAXParser::~XMLSAXParser(){
    ne_xml_destroy(_ne_parser);
}


int XMLSAXParser::parseChuck(const char *partial_string, size_t length){
    int ret = ne_xml_parse(_ne_parser,partial_string,length);
    if(ret != 0){
        if(ret > 0){
            ret =-1;
            const char* ne_parser_err = ne_xml_get_error(_ne_parser);
            throw Davix::DavixException(davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "XML Parsing Error: " + std::string((ne_parser_err)?ne_parser_err:"Unknow ne error"));
        }
        throw Davix::DavixException(davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "Unknow XML parsing error ");
    }
    return ret;
}


int XMLSAXParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    return davParserNotImplemented();
}


int XMLSAXParser::parserCdataCb(int state, const char *cdata, size_t len){
    return davParserNotImplemented();
}

int XMLSAXParser::parserEndElemCb(int state, const char *nspace, const char *name){
    return davParserNotImplemented();
}


int XMLSAXParser::startElemCb(const Chunk & data,
                             const std::vector<XMLSAXParser::Chunk> & attrs){
    return 0;
}

int XMLSAXParser::cdataCb(const Chunk & data){
    return 0;
}

int XMLSAXParser::endElemCb(const Chunk & data){
    return 0;
}

int XMLSAXParser::commentCb(const Chunk & data){
    return 0;
}

} // namespace Davix
