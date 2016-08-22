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

#ifndef DAVIX_DAVXMLPARSER_H
#define DAVIX_DAVXMLPARSER_H


#include <utils/davix_types.hpp>
#include <status/davixstatusrequest.hpp>
#include <xml/davix_ptree.hpp>
#include <ne_xml.h>

#include <utils/davix_fileproperties.hpp>


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
    int parseChunk(const char * partial_string, dav_size_t len);

    inline int parseChunk(const std::string & partial_string){
        return parseChunk(partial_string.c_str(), partial_string.size());
    }

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



class ElementsParser {
public:
    ElementsParser(){}
    virtual ~ElementsParser(){}

    virtual std::deque<FileProperties> & getProperties()=0;

};


class XMLPropParser: public XMLSAXParser, public ElementsParser{
public:
    XMLPropParser(){}
    virtual ~XMLPropParser(){}

};



} // namespace Davix

#endif // DAVIX_DAVXMLPARSER_H
