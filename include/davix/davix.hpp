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

/// main context
#include <davixcontext.hpp>

/// authentication utilities
#include <auth/davixauth.hpp>

/// low level HttpRequest builder
#include <request/httprequest.hpp>

/// request parameters
#include <params/davixrequestparams.hpp>

/// davix uri parser
#include <davixuri.hpp>

/// file API, main API for remote I/O
#include <file/davfile.hpp>

/// posix like API
#include <posix/davposix.hpp>

/// status and error management
#include <status/davixstatusrequest.hpp>

/// logger features
#include <logger/davix_logger.h>



#endif // DAVIX_HPP
