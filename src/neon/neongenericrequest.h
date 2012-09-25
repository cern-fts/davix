#ifndef DAVIX_NEONGENERICREQUEST_H
#define DAVIX_NEONGENERICREQUEST_H

#include <davixhttprequest.h>
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


protected:
    HeaderFieldVec _headers;

};

} // namespace Davix

#endif // DAVIX_NEONGENERICREQUEST_H
