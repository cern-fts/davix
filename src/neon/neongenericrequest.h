#ifndef DAVIX_NEONGENERICREQUEST_H
#define DAVIX_NEONGENERICREQUEST_H

#include <davixhttprequest.h>

namespace Davix {

class NeonGenericRequest : public NGQHttpRequest
{
public:
    NeonGenericRequest(Context* context);
};

} // namespace Davix

#endif // DAVIX_NEONGENERICREQUEST_H
