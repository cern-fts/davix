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

int davParserNotImplemented(Davix::DavixError** err){
    Davix::DavixError::setupError(err, davix_scope_xml_parser(), StatusCode::OperationNonSupported, "the parser callbacks are not setup properly");
    return -1;
}

XMLSAXParser::XMLSAXParser() :  err(NULL), _ne_parser(ne_xml_create())
{
    ne_xml_push_handler(_ne_parser,
                        &InternalDavParser::dav_xml_parser_ne_xml_startelm_cb,
                        &InternalDavParser::dav_xml_ne_xml_cdata_cb,
                        &InternalDavParser::ne_xml_endelm_cb,
                        this);
}

XMLSAXParser::~XMLSAXParser(){
    ne_xml_destroy(_ne_parser);
    Davix::DavixError::clearError(&err);
}


int XMLSAXParser::parseChuck(const char *partial_string, size_t length){
    int ret = ne_xml_parse(_ne_parser,partial_string,length);
    if(ret != 0){
        if(ret > 0){
            ret =-1;
            const char* ne_parser_err = ne_xml_get_error(_ne_parser);
            DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "XML Parsing Error: " + std::string((ne_parser_err)?ne_parser_err:"Unknow ne error"));
        }
        if(!err){
             DavixError::setupError(&err, davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "Unknow XML parsing error ");
        }
    }
    return ret;
}


Davix::DavixError* XMLSAXParser::getLastErr(){

    return ((err)?(err->clone()):(NULL));
}

int XMLSAXParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    return davParserNotImplemented(&err);
}


int XMLSAXParser::parserCdataCb(int state, const char *cdata, size_t len){
    return davParserNotImplemented(&err);
}

int XMLSAXParser::parserEndElemCb(int state, const char *nspace, const char *name){
    return davParserNotImplemented(&err);
}


} // namespace Davix
