#include "davixhttprequest.h"


#include <cstring>

namespace Davix {

NGQHttpRequest::NGQHttpRequest(Context * context) : NGQRequest(context)
{
}


void NGQHttpRequest::set_connexion_timeout(const timespec *conn_timeout){
    memcpy(&(this->conn_timeout), conn_timeout, sizeof(struct timespec));
}

void NGQHttpRequest::set_operation_timeout(const timespec *ops_timeout){
    memcpy(&(this->ops_timeout), ops_timeout, sizeof(struct timespec));
}

} // namespace Davix
