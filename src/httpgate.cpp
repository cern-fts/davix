#include "httpgate.h"

#include <davixhttprequest.h>
#include <abstractsessionfactory.hpp>

namespace Davix {

HttpGate::HttpGate(Context * context) : Gate(context)
{
}

NGQHttpRequest* HttpGate::createRequest(const std::string & uri){
  //  AbstractSessionFactory* facto = context->_intern->
    return NULL;
}

} // namespace Davix




