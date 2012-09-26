#ifndef DAVIX_DAVIXNEONREQUESTSTATUS_H
#define DAVIX_DAVIXNEONREQUESTSTATUS_H


#include <neon/ne_request.h>
#include <neon/ne_session.h>
#include <davixdavstatusrequest.h>


namespace Davix {

class Context;
class NeonGenericRequest;

class NeonRequestStatus : public DavStatusRequest
{
public:
    NeonRequestStatus(Context* context, NeonGenericRequest* request);

private:
    ne_session *    _sess;
    ne_request * _req;
    friend class NeonGenericRequest;
};

} // namespace Davix

#endif // DAVIX_DAVIXNEONREQUESTSTATUS_H
