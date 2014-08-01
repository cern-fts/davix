#include "delegation.hpp"

using namespace Davix;


std::string DavixDelegation::delegate_v2(Context & context, const std::string &dlg_endpoint,
         const Davix::RequestParams& params, const std::string& ucred, const std::string& passwd,
         const std::string& capath,
         int lifetime, Davix::DavixError** err)
{
	DavixError::setupError(err, DELEGATION_SCOPE, StatusCode::DelegationError, "Delegation v2 not implemented");
	return std::string();
}
