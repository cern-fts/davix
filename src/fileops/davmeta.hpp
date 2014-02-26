#ifndef DAVMETA_HPP
#define DAVMETA_HPP

#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <file/davfile.hpp>

namespace Davix{

namespace Meta{


// get all reps from webdav queries
void getReplicas(Context & c, const Uri & r,
                              const RequestParams & params,  std::vector<DavFile> & vec);

dav_ssize_t posixStat(Context & c, const Uri & url, const RequestParams * _params,
                      struct stat* st,
                      DavixError** err);


int deleteResource(Context & c, const Uri & u, const RequestParams & params, DavixError** err);


int makeCollection(Context & c, const Uri & uri, const RequestParams & params, DavixError** err);

/*
  retrieve a webdav propfind stat request to the given url
    @param req : http request where to executethe query
    @return vector of characters of the query content
  */
const char* req_webdav_propfind(HttpRequest* req, DavixError** err);


// utilities
int davix_metalink_header_parser(const std::string & header_key, const std::string & header_value,
                                 const Uri & u_original,
                                 Uri & metalink);


} // Meta

} // Davix




#endif // DAVMETA_HPP
