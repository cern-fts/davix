#ifndef DAVIX_DAVIXSTATUSREQUEST_HPP
#define DAVIX_DAVIXSTATUSREQUEST_HPP

#include <string>
#include <davix_types.h>
#include <status/davix_error.h>
#include <iostream>

/**
  @file davixstatusrequest.hpp
  @author Devresse Adrien, CERN

  @brief C++ Error report system of davix
*/


namespace Davix {

class Context;
class NGQRequest;
struct DavixErrorInternal;

namespace StatusCode{

///
/// @brief Davix status codes
///
/// List of Davix Status code used by \ref Davix::DavixError
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
    isNotADirectory = DAVIX_STATUS_IS_NOT_A_DIRECTORY,
    invalidFileHandle = DAVIX_STATUS_INVALID_FILE_HANDLE,
    alreadyRunning = DAVIX_STATUS_ALREADY_RUNNING,
    authentificationError = DAVIX_STATUS_AUTHENTIFICATION_ERROR,
    loginPasswordError = DAVIX_STATUS_LOGIN_PASSWORD_ERROR,
    credentialNotFound = DAVIX_STATUS_CREDENTIAL_NOT_FOUND,
    permissionRefused = DAVIX_STATUS_PERMISSION_REFUSED,
    fileNotFound = DAVIX_STATUS_FILE_NOT_FOUND,
    IsADirectory = DAVIX_STATUS_IS_A_DIRECTORY,
    SystemError = DAVIX_STATUS_SYSTEM_ERROR,
    FileExist = DAVIX_STATUS_FILE_EXIST
};

const davix_status_t mOk= DAVIX_STATUS_OK;

}

///
///  @brief Davix Error Handler
///
/// Error report system of Davix
/// DavixError has a similar behavior to the glib Error system GError
///
/// Each function which takes a DavixError** as argument can take the value NULL
///
/// Example :
///
///     DavixError *tmp_err = NULL;
///     do_operation(arg1,arg2, &tmp_err)
///     if(tmp_err){ /* test if error occures*/
///         std::cout << tmp_err->getErrMsg() << std::endl;
///         clearError(&tmp_err); // clean error
///     }
///
class DavixError{
public:

    ///
    /// Construct a DavixError object
    ///
    /// @param scope : string parameter representing the scope of the error
    /// @param errCode : Davix Error code, see \ref Davix::StatusCode::Code
    /// @param errMsg : String representation of the error
    DavixError(const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);
    DavixError(const DavixError & e);
    DavixError & operator=(const DavixError & e);
    virtual ~DavixError();


    ///
    /// clone this error in a new dynamically allocated one
    /// need to be delete
    DavixError* clone();

    ///
    /// @return Davix status code of the error
    StatusCode::Code getStatus() const;

    ///
    /// set the status code for this error
    void setStatus(const StatusCode::Code);

    ///
    /// get the string representation of this error
    const std::string & getErrMsg() const;

    ///
    /// set the string representation of this error
    void setErrMsg(const std::string & msg);

    ///
    /// create a new dynamically allocated DavixError Object
    /// if err is NULL, do nothing
    ///
    /// @param scope : string parameter representing the scope of the error
    /// @param errCode : Davix Error code, see \ref Davix::StatusCode::Code
    /// @param errMsg : String representation of the error
    static void setupError(DavixError** err, const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);

    ///
    /// clear the content of the current error and set err to NULL
    static void clearError(DavixError** err);

    ///
    /// propagate the Davix Error Object from oldErr to newErr
    /// OldErr can be consider as free after this operation
    /// erase the error message if newErr is NULL
    ///
    static void propagateError(DavixError** newErr, DavixError* oldErr);

    ///
    /// same than propagateError but add a string prefix in front of the error description
    ///
    /// OldErr can be consider as free after this operation
    /// erase the error message if newErr is NULL
    ///
    static void propagatePrefixedError(DavixError** newErr, DavixError* oldErr, const std::string & prefix);

private:
   DavixErrorInternal * d_ptr;
};


///
/// scope of the davix stat part
//
std::string davix_scope_stat_str();
std::string davix_scope_davOps_str();
std::string davix_scope_mkdir_str();
std::string davix_scope_directory_listing_str();
std::string davix_scope_http_request();
std::string davix_scope_xml_parser();
std::string davix_scope_uri_parser();
std::string davix_scope_io_cache();


} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
