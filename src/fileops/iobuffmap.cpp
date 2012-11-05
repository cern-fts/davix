#include "iobuffmap.hpp"

namespace Davix {

IOBuffMap::IOBuffMap(Context & c, const Uri & uri, const RequestParams & params) : _c(c), _uri(uri), _params(params)
{
}

bool IOBuffMap::open(int flags, DavixError **err){
    return false;
}

ssize_t IOBuffMap::read(void *buf, size_t count, DavixError **err){
    return -1;
}

ssize_t IOBuffMap::write(const void *buf, size_t count, DavixError **err){
    return -1;
}

off_t IOBuffMap::lseek(off_t offset, int flags, DavixError **err){
    return -1;
}

ssize_t IOBuffMap::putOps(const void *buf, size_t count, off_t offset, DavixError **err){
    return -1;
}

ssize_t IOBuffMap::getOps(void *buf, size_t count, off_t offset, DavixError **err){
    return -1;
}

} // namespace Davix
