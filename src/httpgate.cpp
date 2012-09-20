#include "httpgate.h"

#include <davixhttprequest.h>

namespace Davix {

HttpGate::HttpGate(Context * context) : Gate(context)
{
}

NGQHttpRequest* HttpGate::createRequest(const std::string & uri){

}

} // namespace Davix




