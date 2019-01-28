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

#ifndef S3DELETEPARSER_HPP
#define S3DELETEPARSER_HPP

#include <deque>

#include <davix_internal.hpp>
#include <xml/davxmlparser.hpp>
#include <utils/davix_fileproperties.hpp>
#include <string.h>

namespace Davix{

class S3DeleteParser :  public XMLPropParser
{
public:
    struct Internal;
    S3DeleteParser();

    virtual ~S3DeleteParser();

    virtual std::deque<FileProperties> & getProperties(); // not used
    std::deque<FileDeleteStatus> & getDeleteStatus();


protected:
    virtual int parserStartElemCb(int parent, const char *nspace, const char *name, const char **atts);
    virtual int parserCdataCb(int state, const char *cdata, size_t len);
    virtual int parserEndElemCb(int state, const char *nspace, const char *name);


private:
    std::unique_ptr<Internal> d_ptr;
};

}

#endif // S3DELETEPARSER_HPP
