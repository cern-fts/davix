/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) 2013  Adrien Devresse <adrien.devresse@cern.ch>, CERN
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

#ifndef DAVIX_FILEPROPERTIES_H
#define DAVIX_FILEPROPERTIES_H

#include <davix_internal_config.hpp>
#include <string>
#include <vector>

#include <file/davix_file_info.hpp>
#include <string_utils/stringutils.hpp>


namespace Davix {



struct FileProperties
{
    FileProperties() :
        filename(),
        req_status(0),
        info(){}

    std::string filename;
    unsigned int  req_status; /* status code of the request associated ( ex: http 200) */

    // stat() metadata
    StatInfo info;

    inline void clear(){
        info = StatInfo();
        filename.clear();
        req_status = 0;
    }


};

} // namespace Davix

#endif // DAVIX_FILEPROPERTIES_H
