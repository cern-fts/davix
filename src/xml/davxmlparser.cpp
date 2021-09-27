/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
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


int XMLSAXParser::parseChunk(const char *partial_string, dav_size_t length){
    int ret = ne_xml_parse(_ne_parser,partial_string,length);
    if(ret != 0){
        if(ret > 0){
            ret =-1;
            const char* ne_parser_err = ne_xml_get_error(_ne_parser);
            throw Davix::DavixException(davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "XML Parsing Error: " + std::string((ne_parser_err)?ne_parser_err:"Unknown ne error"));
        }
        throw Davix::DavixException(davix_scope_xml_parser(), StatusCode::WebDavPropertiesParsingError, "Unknown XML parsing error ");
    }
    return ret;
}


int XMLSAXParser::parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts){
    (void) parent;
    (void) nspace;
    (void) name;
    (void) atts;
    return davParserNotImplemented();
}


int XMLSAXParser::parserCdataCb(int state, const char *cdata, size_t len){
    (void) state;
    (void) cdata;
    (void) len;
    return davParserNotImplemented();
}

int XMLSAXParser::parserEndElemCb(int state, const char *nspace, const char *name){
    (void) state;
    (void) nspace;
    (void) name;
    return davParserNotImplemented();
}


int XMLSAXParser::startElemCb(const Chunk & data,
                             const std::vector<XMLSAXParser::Chunk> & attrs){
    (void) data;
    (void) attrs;
    return -1;
}

int XMLSAXParser::cdataCb(const Chunk & data){
    (void) data;
    return -1;
}

int XMLSAXParser::endElemCb(const Chunk & data){
    (void) data;
    return 0;
}

int XMLSAXParser::commentCb(const Chunk & data){
    (void) data;
    return 0;
}

} // namespace Davix
