#ifndef DAVIX_DAVIXHTTPREQUEST_H
#define DAVIX_DAVIXHTTPREQUEST_H

#include <davixrequest.h>

namespace Davix {

class NGQRequest;
class Context;

class NGQHttpRequest : public NGQRequest
{
public:
    NGQHttpRequest(Context* context);
};

} // namespace Davix

#endif // DAVIX_DAVIXHTTPREQUEST_H
