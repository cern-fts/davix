#include "request.hpp"
#include <status/davixstatusrequest.hpp>

namespace Davix {

Request::Request()
{
}

int Request::try_set_pkcs12_cert(const char *filename_pkcs12, const char *passwd, DavixError** err){
    DavixError::setupError(err, davix_scope_http_request(), StatusCode::OperationNonSupported, "Not implemented");
    return -1;
}

int Request::try_set_login_passwd(const char *login, const char *passwd, DavixError** err){
    DavixError::setupError(err, davix_scope_http_request(), StatusCode::OperationNonSupported, "Not implemented");
    return -1;
}

} // namespace Davix


