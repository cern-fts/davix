#ifndef DAVOPS_HPP
#define DAVOPS_HPP

#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>

namespace Davix{


int DavOpsDelete(Context &, const RequestParams & params, const Uri & uri, DavixError** err);

}

#endif // DAVOPS_HPP
