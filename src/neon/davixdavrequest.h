#ifndef DAVIX_DAVIXDAVREQUEST_H
#define DAVIX_DAVIXDAVREQUEST_H

#include <davixhttprequest.h>

namespace Davix {

class NGQHttpRequest;
class Context;
class Uri;

class DavRequest : public NGQHttpRequest
{
public:
    DavRequest(Context* context, const Uri & my_uri);
};

} // namespace Davix

#endif // DAVIX_DAVIXDAVREQUEST_H
