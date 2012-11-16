#include "davposix.hpp"

namespace Davix {

DavPosix::DavPosix(Context* _context) :
    context(_context),
    _timeout(180),
    _s_buff(2048),
    d_ptr(NULL)

{

}


DavPosix::~DavPosix(){

}

} // namespace Davix
