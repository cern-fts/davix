#include "request.h"

namespace Davix {

Request::Request()
{
}

void Request::try_set_pkcs12_cert(const char *filename_pkcs12, const char *passwd){
    throw Glib::Error(Glib::Quark("Request::try_set_pkcs12_cert"), DAVIX_ERROR_ENOTSUPPORT, "Not implemented");
}

} // namespace Davix


