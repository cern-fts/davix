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

#ifndef DAVIX_FILE_TYPES_HPP
#define DAVIX_FILE_TYPES_HPP

#include <memory>
#include <utils/davix_types.hpp>
#include <utils/davix_uri.hpp>

/**
  @file davix_file_types.hpp
  @author Devresse Adrien


  @brief davix file related type declarations
*/


// file descriptor declaration
struct Davix_dir_handle;
struct Davix_fd;

typedef struct Davix_dir_handle DAVIX_DIR;
typedef struct Davix_fd DAVIX_FD;

namespace Davix{


/// @struct DavIOVecInput
/// @brief input parameters for vector operations in Davix
struct DAVIX_EXPORT DavIOVecInput{
    void* diov_buffer;                    /**< buffer, in case of read : destination buffer, in case of write : source buffer */
    dav_off_t diov_offset;                /**< initial offset taken from the source */
    dav_size_t diov_size;                 /**< size of the data requested */
};

/// @struct DavIOVecOuput
/// @brief result of vector operations in Davix
struct DAVIX_EXPORT DavIOVecOuput{
    void* diov_buffer;                    /**< pointer to the buffer used for this fragment */
    dav_ssize_t diov_size;                /**< size of the data returned, -1 if error */
};


/// @enum advise_t
/// Information about the next type of operation executed
/// AdviseAuto : default operation, no optimization
/// AdviseSequentialRead : optimize next operation for sequential read/write
/// AdviseRandomRead: optimize next operation for random position read/write
enum DAVIX_EXPORT advise_t{
    AdviseAuto=0x00,
    AdviseSequential,
    AdviseRandom,

};

///
/// @brief QuotaInfo struct
/// @struct QuotaInfo
/// handler to retrieve quota information
///

class QuotaInfoHandler;

class QuotaInfo {
friend class QuotaInfoHandler;
public:
    struct Internal;
    QuotaInfo();
    ~QuotaInfo();
    dav_size_t getUsedBytes();
    dav_size_t getFreeSpace();
private:
    std::shared_ptr<Internal> d_ptr;
};


///
/// @brief StatInfo struct
/// @struct StatInfo
/// container for base file meta-data, plateform agnostic stat struct
///
struct StatInfo{
    StatInfo(): size(0), nlink(0), mode(0), atime(0), mtime(0), ctime(0), owner(0), group(0) {
    }

    /// size in bytes of the resource
    dav_size_t size;
    /// number of links to the resource
    /// optional
    dav_ssize_t nlink;
    /// POSIX rights of the resource
    /// optional, supported with some Webdav servers
    mode_t mode;
    /// access time
    time_t atime;
    /// modification time
    time_t mtime;
    /// creation time
    time_t ctime;
    /// owner UID
    uid_t owner;
    /// group UID
    gid_t group;

    /// struct converter from POSIX stat
    inline void fromPosixStat(const struct stat & st){
        mode = static_cast<mode_t>(st.st_mode);
        atime = static_cast<time_t>(st.st_atime);
        mtime = static_cast<time_t>(st.st_mtime);
        ctime = static_cast<time_t>(st.st_ctime);
        size =  static_cast<dav_size_t>(st.st_size);
        nlink = static_cast<dav_size_t>(st.st_nlink);
        owner = static_cast<uid_t>(st.st_uid);
        group = static_cast<gid_t>(st.st_gid);
    }

    /// struct converter to POSIX stat
    inline struct stat & toPosixStat(struct stat & st){
        st.st_mode = static_cast<mode_t>(mode);
        st.st_atime = static_cast<time_t>(atime);
        st.st_mtime = static_cast<time_t>(mtime);
        st.st_ctime = static_cast<time_t>(ctime);
        st.st_size =  static_cast<off_t>(size);
        st.st_nlink = static_cast<nlink_t>(nlink);
        st.st_uid = static_cast<uid_t>(owner);
        st.st_gid = static_cast<gid_t>(group);
        return st;
    }
};



} // Davix


#endif // DAVIX_FILE_TYPES_HPP
