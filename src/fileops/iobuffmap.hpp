#pragma once
#ifndef DAVIX_IOBUFFMAP_HPP
#define DAVIX_IOBUFFMAP_HPP

#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>


namespace Davix {

//
// Internal POSIX like to HTTP RW operation mapper
// provides facilities for caching
//
class IOBuffMap
{
public:
    IOBuffMap(Context & c, const Uri & uri, const RequestParams & params);

private:
    Context & _c;
    Uri _uri;
    RequestParams _params;
};

} // namespace Davix

#endif // DAVIX_IOBUFFMAP_HPP
