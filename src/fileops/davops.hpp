#ifndef DAVOPS_HPP
#define DAVOPS_HPP

#include <memory>

#include <davixcontext.hpp>
#include <davixuri.hpp>
#include <params/davixrequestparams.hpp>

#include <neon/neonsession.hpp>

namespace Davix{


int DavOpsDelete(Context & c, const RequestParams & params, const Uri & uri, DavixError** err);


}

#endif // DAVOPS_HPP
