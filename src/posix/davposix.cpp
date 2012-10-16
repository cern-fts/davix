#include "davposix.hpp"

namespace Davix {

DavPosix::DavPosix(Context* context)
{
    this->context = context;
    _s_buff= 2048;
    _timeout =180;
}


DavPosix::~DavPosix(){

}

} // namespace Davix
