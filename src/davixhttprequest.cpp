#include "davixhttprequest.h"
#include <httpstatusrequest.h>

#include <cstring>

namespace Davix {

NGQHttpRequest::NGQHttpRequest(Context* context, const Uri & my_uri) : NGQRequest(context)
{
    this->_uri =my_uri;
}


HttpStatusRequest* NGQHttpRequest::executeRequest(){

}



} // namespace Davix
