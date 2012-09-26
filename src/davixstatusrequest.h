#ifndef DAVIX_DAVIXSTATUSREQUEST_H
#define DAVIX_DAVIXSTATUSREQUEST_H

#include <davixcontext.h>
#include <davixrequest.h>

namespace Davix {

namespace StatusCode{

enum Code{
    OK,
    PartialDone,
    WebDavPropertiesParsingError,
    UriParsingError,
    NameResolutionFailure,
    ConnexionProblem,
    RedirectionNeeded,
    ConnexionTimeout,
    OperationTimeout,
    OperationNonSupported,
    UnknowError
};

}

class StatusRequest
{
public:
    StatusRequest(Context* context);

    inline StatusCode::Code getStatus() const{
        return code;
    }

    inline const std::string & getErrMsg() const{
        return err_msg;
    }

protected:
    Context* context;
    NGQRequest* parent_request;
    StatusCode::Code code;
    std::string err_msg;
};

} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
