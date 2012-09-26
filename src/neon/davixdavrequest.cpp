#include "davixdavrequest.h"

namespace Davix {

DavRequest::DavRequest(Context* context, const Uri & my_uri) : NGQHttpRequest(context, my_uri)
{
}

} // namespace Davix
