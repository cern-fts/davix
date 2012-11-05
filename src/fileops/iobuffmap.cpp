#include "iobuffmap.hpp"

namespace Davix {

IOBuffMap::IOBuffMap(Context & c, const Uri & uri, const RequestParams & params) : _c(c), _uri(uri), _params(params)
{
}

} // namespace Davix
