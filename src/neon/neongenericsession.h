#ifndef DAVIX_NEONGENERICSESSION_H
#define DAVIX_NEONGENERICSESSION_H

#include <davixhttprequest.h>

namespace Davix {

class NeonGenericSession : public NGQHttpRequest
{
public:
    NeonGenericSession(Context* context);
};

} // namespace Davix

#endif // DAVIX_NEONGENERICSESSION_H
