#ifndef DAVIX_DELEGATION_HPP
#define DAVIX_DELEGATION_HPP

#include <davix.hpp>
#include <string>

namespace Davix {

extern const std::string DELEGATION_SCOPE;

// Need a class so it can be frieds with Session
class DavixDelegation {
public:
    static std::string delegate(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, Davix::DavixError** err);

private:
    static bool get_credentials(const RequestParams& params,
            std::string& ucred, std::string& passwd, std::string& capath,
            int *lifetime, DavixError** err);

    static std::string delegate_v1(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, const std::string& ucred, const std::string& passwd,
            const std::string& capath,
            int lifetime, Davix::DavixError** err);

    static std::string delegate_v2(Context & context, const std::string &dlg_endpoint,
            const Davix::RequestParams& params, const std::string& ucred, const std::string& passwd,
            const std::string& capath,
            int lifetime, Davix::DavixError** err);
};

}

#endif
