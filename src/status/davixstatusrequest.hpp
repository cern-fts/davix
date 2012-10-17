#ifndef DAVIX_DAVIXSTATUSREQUEST_HPP
#define DAVIX_DAVIXSTATUSREQUEST_HPP

#include <string>
#include <davix_types.h>

/**
  @file davixstatusrequest.hpp
  @author Devresse Adrien

  @brief C++ Error report system of davix
*/


namespace Davix {

class Context;
class NGQRequest;
struct DavixErrorInternal;

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

/**
  @brief Davix Error Handler
*/
class DavixError{
public:
    DavixError(const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);
    virtual ~DavixError();



    StatusCode::Code getStatus() const;

    void setStatus(const StatusCode::Code);

    const std::string & getErrMsg() const;

    void setErrMsg(const std::string & msg);

    static void setupError(DavixError** err, const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);

    static void clearError(DavixError** err){
        if(err && *err){
            delete *err;
            *err = NULL;
        }
    }

private:
   DavixErrorInternal * d_ptr;
};



} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
