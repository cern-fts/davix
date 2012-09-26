#ifndef DAVIX_NEONGENERICREQUEST_H
#define DAVIX_NEONGENERICREQUEST_H

#include <davixhttprequest.h>
#include <neon/davixneonrequeststatus.h>
#include <neon/ne_session.h>
#include <neon/ne_redirect.h>
#include <neon/ne_request.h>
#include <vector>
#include <string>

typedef std::pair<std::string, std::string> HeaderField;
typedef std::vector<HeaderField>  HeaderFieldVec;

namespace Davix {

class NeonRequestStatus;

class NeonGenericRequest : public NGQHttpRequest
{
public:
    NeonGenericRequest(Context* context, const Uri & uri);

    virtual void addHeaderField(const std::string &field, const std::string &value);

    virtual NeonRequestStatus* executeRequest();

protected:
    HeaderFieldVec _headers;

    void configure_sess(ne_session* sess);

};

} // namespace Davix

#endif // DAVIX_NEONGENERICREQUEST_H
