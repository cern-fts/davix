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

#ifndef DAVIX_FILEPROPERTIES_H
#define DAVIX_FILEPROPERTIES_H

#include <davix_internal_config.hpp>
#include <string>
#include <vector>

#include <file/davix_file_info.hpp>
#include <utils/stringutils.hpp>


namespace Davix {


struct QuotaInfo::Internal {
    dav_size_t free_space;
    dav_size_t used_bytes;

    Internal() {
        free_space = 0;
        used_bytes = 0;
    }
};

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
    QuotaInfo::Internal quota;

    inline void clear(){
        info = StatInfo();
        filename.clear();
        req_status = 0;
    }


};

struct FileDeleteStatus
{
    FileDeleteStatus() :
        filename(),
        message(),
        error_code(),
        req_status(0),
        error(false){}

    std::string filename;
    std::string message;
    std::string error_code; // for when server return a text code
    unsigned int  req_status; /* status code of the request associated ( ex: http 200) */
    bool error;

    inline void clear(){
        filename.clear();
        message.clear();
        error_code.clear();
        req_status = 0;
        error = false;
    }
};

} // namespace Davix

#endif // DAVIX_FILEPROPERTIES_H
