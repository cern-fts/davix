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

#ifndef DAVIX_HPP
#define DAVIX_HPP



///
/// @file davix.hpp
/// @author Devresse Adrien
///
///
/// @brief C++ API of Davix
///  Davix is a high level HTTP/Webdav library
///  for file management and file access.
///
/// You need to create a context before any operations
///


#ifndef __DAVIX_INSIDE__
#define __DAVIX_INSIDE__
#endif

#ifndef DAVIX_EXPORT
#define DAVIX_EXPORT
#endif

#include <davix_file_types.hpp>
#include <features.hpp>

/// main context
#include <davixcontext.hpp>

/// authentication utilities
#include <auth/davixauth.hpp>

/// low level HttpRequest builder
#include <request/httprequest.hpp>

/// request parameters
#include <params/davixrequestparams.hpp>

/// davix uri parser
#include <utils/davix_uri.hpp>

/// file API, main API for remote I/O
#include <file/davfile.hpp>

/// posix like API
#include <file/davposix.hpp>

/// status and error management
#include <status/davixstatusrequest.hpp>

/// logger features
#include <utils/davix_logger.hpp>

/// REST utilities functions
#include <utils/davix_s3_utils.hpp>
#include <utils/davix_azure_utils.hpp>

/// generic utils
#include <utils/davix_misc_utils.hpp>

/// hooks for extensions
#include <hooks/davix_hooks.hpp>

// The usage of this header :
// - promote Comic Sans
// - create black holes
// - kill kitten
#include <compat/deprecated.hpp>

// third party copy
// Note this is an extension supported
// by a couple of http implementations only
// i.e. lcgdm-dav
#include <copy/davixcopy.hpp>


#endif // DAVIX_HPP
