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


#ifndef DAVIX_FILE_TYPES_HPP
#define DAVIX_FILE_TYPES_HPP

#include <utils/davix_types.hpp>
#include <utils/davix_uri.hpp>

/**
  @file davix_file_types.hpp
  @author Devresse Adrien


  @brief davix file related type declarations
*/


// global file descriptor declaration
typedef struct Davix_dir_handle DAVIX_DIR;
typedef struct Davix_fd DAVIX_FD;

struct Davix_fd;
struct Davix_dir_handle;


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


} // Davix


#endif // DAVIX_FILE_TYPES_HPP
