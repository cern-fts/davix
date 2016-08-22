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
