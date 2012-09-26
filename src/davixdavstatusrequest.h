#ifndef DAVIX_DAVIXDAVSTATUSREQUEST_H
#define DAVIX_DAVIXDAVSTATUSREQUEST_H

#include <davixdavstatusrequest.h>

#include <davixhttprequest.h>

namespace Davix {

class DavStatusRequest : public HttpStatusRequest
{
public:
    DavStatusRequest(Context * context, NGQHttpRequest* request);
};

} // namespace Davix

#endif // DAVIX_DAVIXDAVSTATUSREQUEST_H
