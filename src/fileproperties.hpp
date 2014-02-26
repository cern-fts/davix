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

#include <config.h>
#include <string>
#include <vector>


namespace Davix {



struct FileProperties
{
    FileProperties();
    std::string filename;
    unsigned int  req_status; /* status code of the request associated ( ex: http 200) */

    nlink_t   nlink;
    uid_t     uid;    /* unix user id */
    gid_t     gid;   /* unix group id */
    off_t     size;  /* total size, in bytes */
    mode_t    mode;

    time_t    atime;   /* time of last access */
    time_t    mtime;   /* time of last modification */
    time_t    ctime;   /* time of last status change */

    inline void clear(){
        nlink = req_status =  gid = uid = size =0;
        mode = atime = mtime = ctime = 0;
        filename=std::string();

    }

};

} // namespace Davix

#endif // DAVIX_FILEPROPERTIES_H
