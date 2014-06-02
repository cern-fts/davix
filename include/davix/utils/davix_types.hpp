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


#ifndef DAVIX_TYPES_HPP
#define DAVIX_TYPES_HPP

#include <string>
#include <iostream>
#include <vector>
#include <stack>
#include <deque>
#include <typeinfo>
#include <algorithm>
#include <functional>

#include <stdint.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp should be included."
#endif


// type decl
#ifndef DAVIX_EXPORT
#define DAVIX_EXPORT __attribute__((visibility("default")))
#endif

struct Davix_context;
struct Davix_error;

// alias
typedef struct Davix_context* davix_sess_t;
typedef struct Davix_error* davix_error_t;
typedef struct davix_file_desc_s* davix_file_desc_t;
typedef struct davix_auth_st* davix_auth_t;
typedef struct davix_request_params* davix_params_t;

namespace Davix{
    // Davix namespace declaration
    typedef std::vector< std::pair <std::string, std::string> > HeaderVec;

} // Davix

// define CXX11 support
#if ( (__cplusplus == 201103L) || (defined (DAVIX_FORCE_CXX11)) )
#define DAVIX_CX11_SUPPORT
#endif


// enable TR1 for non CX11 compilation
#ifndef DAVIX_CX11_SUPPORT

#include <tr1/functional>

// bind tr1 to std for old compiler without CXX11
namespace std{
    using namespace std::tr1;
}
#endif

// Disable all non C++03 features
//#define DAVIX_STD_CXX03

// Davix Large File Support
#if  ( __WORDSIZE == 32 ) || \
        ( SIZE_MAX ==  (4294967295U) ) || \
        ( defined __WIN32 )

typedef uint64_t dav_off_t;
typedef uint64_t dav_size_t;
typedef int64_t dav_ssize_t;
#else
typedef off_t dav_off_t;
typedef size_t dav_size_t;
typedef ssize_t dav_ssize_t;
#endif

// block size
#define DAVIX_BLOCK_SIZE 4096
#define DAVIX_MAX_BLOCK_SIZE 16777216

//
// davix preproc facilities
//
#undef DAVIX_C_DECL_BEGIN
#undef DAVIX_C_DECL_END
#ifdef __cplusplus
#define DAVIX_C_DECL_BEGIN \
        extern "C" {
#define DAVIX_C_DECL_END }
#else
#define DAVIX_C_DECL_BEGIN  // void
#define DAVIX_C_DECL_END    // void
#endif




/// @class NonCopyable
/// @brief Simple NonCopyableProtector
class NonCopyable{
protected:
   NonCopyable() {}
    ~NonCopyable() {}
private:  // emphasize the following members are private
   NonCopyable( const NonCopyable& );
   const NonCopyable& operator=( const NonCopyable& );
};

#endif // DAVIX_TYPES_HPP
