#ifndef DAVIX_DAVIXSTATUSREQUEST_HPP
#define DAVIX_DAVIXSTATUSREQUEST_HPP

#include <string>
#include <davix_types.h>

namespace Davix {

class Context;
class NGQRequest;

namespace StatusCode{

///
/// Davix status codes
///
enum Code{
    OK = DAVIX_STATUS_OK,
    PartialDone = DAVIX_STATUS_PARTIAL_DONE,
    WebDavPropertiesParsingError = DAVIX_STATUS_WEBDAV_PROPERTIES_PARSING_ERROR,
    UriParsingError = DAVIX_STATUS_URI_PARSING_ERROR,
    SessionCreationError = DAVIX_STATUS_SESSION_CREATION_ERROR,
    NameResolutionFailure= DAVIX_STATUS_NAME_RESOLUTION_FAILURE,
    ConnexionProblem = DAVIX_STATUS_CONNEXION_PROBLEM,
    RedirectionNeeded = DAVIX_STATUS_REDIRECTION_NEEDED,
    ConnexionTimeout= DAVIX_STATUS_CONNEXION_TIMEOUT,
    OperationTimeout= DAVIX_STATUS_OPERATION_TIMEOUT,
    OperationNonSupported= DAVIX_STATUS_OPERATION_NOT_SUPPORTED,
    UnknowError= DAVIX_STATUS_UNKNOW_ERROR,
    isNotADirectory = DAVIX_STATUS_IS_NOT_A_DIRECTORY
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
