#include "davixhttprequest.h"


#include <cstring>

namespace Davix {

NGQHttpRequest::NGQHttpRequest(Context* context, const Uri & my_uri) : NGQRequest(context)
{
    this->_uri =my_uri;
}



} // namespace Davix
