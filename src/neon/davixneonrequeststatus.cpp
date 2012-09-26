#include "davixneonrequeststatus.h"
#include <neon/neongenericrequest.h>

namespace Davix {

NeonRequestStatus::NeonRequestStatus(Context* context, NeonGenericRequest* request) : DavStatusRequest(context, request)
{

}

} // namespace Davix
