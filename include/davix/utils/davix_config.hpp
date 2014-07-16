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
#ifndef DAVIX_CONFIG
#define DAVIX_CONFIG

#include <functional>

#ifndef __DAVIX_INSIDE__
#error "Only davix.hpp should be included."
#endif


// type decl
#ifndef DAVIX_EXPORT
#define DAVIX_EXPORT __attribute__((visibility("default")))
#endif

//
// detect CXX11 support
//
#if ( (__cplusplus > 199711L) \
     || (defined (DAVIX_FORCE_CXX11)) \
     || (defined __GXX_EXPERIMENTAL_CXX0X__) )
#   ifndef __DAVIX_CX11_SUPPORT
#       define __DAVIX_CX11_SUPPORT
#   endif
#endif

//
// detect TR1 support
//
#if ( (defined DAVIX_FORCE_TR1) \
      || (defined __GNUG__) \
      || (defined HAVE_TR1_SUPPORT))
#   ifndef __DAVIX_TR1_SUPPORT
#       define __DAVIX_TR1_SUPPORT
#endif

#endif


//
// include tr1 namespace
//

#if (!(defined __DAVIX_CX11_SUPPORT) \
    && (defined __DAVIX_TR1_SUPPORT))
#       include <tr1/functional>

namespace std{
    using namespace std::tr1;
}

#endif

//
// enable STD_FUNCTION_DECL
//
#if ((defined __DAVIX_CX11_SUPPORT) \
    || (defined __DAVIX_TR1_SUPPORT) \
    || (defined HAVE_STD_FUNCTION))
#   ifndef __DAVIX_HAS_STD_FUNCTION
#       define __DAVIX_HAS_STD_FUNCTION
#   endif
#endif


// Davix Large File Support for plateforms != 64 bits
#if  ( __WORDSIZE == 32 ) || \
        ( SIZE_MAX ==  (4294967295U) ) || \
        ( defined __WIN32 )
#ifndef __DAVIX_LFS_SUPPORT
#define __DAVIX_LFS_SUPPORT
#endif
#endif


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




#endif // DAVIX_TYPES_HPP
