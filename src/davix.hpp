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


/// main context
#include <davixcontext.hpp>

/// authentication utilities
#include <auth/davixauth.hpp>

/// low level HttpRequest builder
#include <httprequest.hpp>

/// request parameters
#include <params/davixrequestparams.hpp>

/// davix uri parser
#include <davixuri.hpp>

/// posix like API
#include <posix/davposix.hpp>

/// status and error management
#include <status/davixstatusrequest.hpp>



#endif // DAVIX_HPP
