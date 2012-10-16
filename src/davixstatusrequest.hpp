#ifndef DAVIX_DAVIXSTATUSREQUEST_H
#define DAVIX_DAVIXSTATUSREQUEST_H

#include <string>

namespace Davix {

class Context;
class NGQRequest;

namespace StatusCode{

enum Code{
    OK,
    PartialDone,
    WebDavPropertiesParsingError,
    UriParsingError,
    SessionCreationError,
    NameResolutionFailure,
    ConnexionProblem,
    RedirectionNeeded,
    ConnexionTimeout,
    OperationTimeout,
    OperationNonSupported,
    UnknowError,
    isNotADirectory
};

}

class StatusOperation{
public:

    inline StatusCode::Code getStatus() const{
        return code;
    }

    inline const std::string & getErrMsg() const{
        return err_msg;
    }

protected:
    StatusCode::Code code;
    std::string err_msg;
};

class StatusRequest : public StatusOperation
{
public:
    StatusRequest(Context* context, NGQRequest* request);
    virtual ~StatusRequest(){}



protected:
    Context* context;
    NGQRequest* parent_request;

};

} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
