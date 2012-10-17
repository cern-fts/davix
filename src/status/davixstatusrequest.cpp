#include "davixstatusrequest.hpp"

namespace Davix {

StatusRequest::StatusRequest(Context* context, NGQRequest* req)
{
    this->context= context;
    this->parent_request = req;
}

} // namespace Davix
