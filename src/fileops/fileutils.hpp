#ifndef DAVIX_FILEUTILS_HPP
#define DAVIX_FILEUTILS_HPP

#include <httprequest.hpp>
#include <status/davixstatusrequest.hpp>

namespace Davix {


// take a HTTP request status and convert file status to common errcode
int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err);


} // namespace Davix

#endif // DAVIX_FILEUTILS_HPP
