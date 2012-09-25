#include "neongenericrequest.h"

namespace Davix {

NeonGenericRequest::NeonGenericRequest(Context* context, const Uri & uri): NGQHttpRequest(context, uri)
{
}

void NeonGenericRequest::addHeaderField(const std::string &field, const std::string &value){
    _headers.push_back(HeaderField(field, value));
}

} // namespace Davix
