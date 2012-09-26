#ifndef DAVIX_HTTPSTATUSREQUEST_H
#define DAVIX_HTTPSTATUSREQUEST_H

#include <davixstatusrequest.h>


namespace Davix {

class Context;
class NGQHttpRequest;

class HttpStatusRequest : public StatusRequest
{
public:
    HttpStatusRequest(Context * context, NGQHttpRequest* request);
};

} // namespace Davix

#endif // DAVIX_HTTPSTATUSREQUEST_H
