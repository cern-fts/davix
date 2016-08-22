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

#ifndef DAVIX_DAVDELETEXMLPARSER_HPP
#define DAVIX_DAVDELETEXMLPARSER_HPP


#include <deque>
#include <xml/davxmlparser.hpp>
#include <string.h>

namespace Davix {


class DavDeleteXMLParser : public XMLPropParser
{
public:
    struct DavxDeleteXmlIntern;
    DavDeleteXMLParser();
    virtual ~DavDeleteXMLParser();

    virtual std::deque<FileProperties> & getProperties();


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);


private:
    DavxDeleteXmlIntern* d_ptr;
};

/*
inline bool match_element(const char* c_origin, const char* c_pattern){ // C style, critical function
    return strcmp(c_origin,c_pattern) ==0;
}
*/
} // namespace Davix

#endif // DAVIX_DAVDELETEXMLPARSER_HPP
