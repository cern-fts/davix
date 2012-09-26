#ifndef DAVIX_DAVIXNEONREQUESTSTATUS_H
#define DAVIX_DAVIXNEONREQUESTSTATUS_H


#include <davixdavstatusrequest.h>

namespace Davix {

class Context;
class NeonGenericRequest;

class NeonRequestStatus : public DavStatusRequest
{
public:
    NeonRequestStatus(Context* context, NeonGenericRequest* request);
};

} // namespace Davix

#endif // DAVIX_DAVIXNEONREQUESTSTATUS_H
