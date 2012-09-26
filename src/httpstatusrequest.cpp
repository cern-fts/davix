#include "httpstatusrequest.h"
#include <davixhttprequest.h>

namespace Davix {

HttpStatusRequest::HttpStatusRequest(Context * context, NGQHttpRequest* request) : StatusRequest(context, request)
{
}

} // namespace Davix
