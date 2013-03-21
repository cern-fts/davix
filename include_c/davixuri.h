#pragma once
#ifndef DAVIX_DAVIXURI_H
#define DAVIX_DAVIXURI_H

#include <string.h>
#include <davix_types.h>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


DAVIX_C_DECL_BEGIN

///  @file davixuri.h
///  @author Devresse Adrien
///
///  @brief C URI utilities functions of davix

typedef struct davix_uri_s* davix_uri_t;

///
/// create a new davix uri objet
/// @param url to parse
/// @return davix_uri_object
davix_uri_t davix_uri_new(const char* url);

///
/// davix uri parsing
///
davix_uri_t davix_uri_copy(davix_uri_t orig_uri);


///
/// release memory of an existing davix uri object
/// @param duri : davix uri to free
void davix_uri_free(davix_uri_t duri);


///
/// get uri port number
/// @param duri : davix uri
int davix_uri_get_port(davix_uri_t duri);

///
/// get a string repsentation of the parser uri
/// @param duri : davix uri
/// @return string representation of the uri. if the uri is not valid, return NULL
const char* davix_uri_get_string(davix_uri_t duri);

///
/// get the scheme of the uri
/// @param duri : davix uri
/// @return string representation of the scheme. if the uri is not valid, return NULL
const char* davix_uri_get_protocol(davix_uri_t duri);

///
/// get a concatenation of the path + the query of the uri
/// @param duri : davix uri
/// @return string representation of the path + query. if the uri is not valid, return NULL
const char* davix_uri_get_path_and_query(davix_uri_t duri);

///
/// get the host of the uri
/// @param duri : davix uri
/// @return string representation of the path + query. if the uri is not valid, return NULL
const char* davix_uri_get_host(davix_uri_t duri);

///
/// get the path of the uri without the query arguments
/// @param duri : davix uri
/// @return string representation of the path. if the uri is not valid, return NULL
const char* davix_uri_get_path(davix_uri_t duri);

///
/// get the status of the parsing for the current query
///
/// @return see Davix StatusCode for more information
davix_status_t davix_uri_get_status(davix_uri_t duri);

DAVIX_C_DECL_END

#endif // DAVIX_DAVIXURI_H
