#include "davixhttprequest.h"
#include <httpstatusrequest.h>

#include <cstring>

namespace Davix {

NGQHttpRequest::NGQHttpRequest(Context* context, const Uri & my_uri) : NGQRequest(context)
{
    this->_uri =my_uri;
    _body = NULL;
    _body_size = 0;
}



void NGQHttpRequest::setRequestContent(uint8_t *body_content, size_t size_body){
    _body = body_content;
    _body_size = size_body;
}



} // namespace Davix
