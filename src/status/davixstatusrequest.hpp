#ifndef DAVIX_DAVIXSTATUSREQUEST_HPP
#define DAVIX_DAVIXSTATUSREQUEST_HPP

#include <string>
#include <davix_types.h>
#include <iostream>

/**
  @file davixstatusrequest.hpp
  @author Devresse Adrien, CERN

  @brief C++ Error report system of davix
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix {

class Context;
class NGQRequest;
struct DavixErrorInternal;

namespace StatusCode{

/// @typedef StatusCode::Code
/// @brief C++ Davix status codes
/// equal to davix_status_t
///
typedef davix_status_t Code;

//
// Davix::statusCode and davix_status_t are
// the same error codes.
//
// Davix::statusCode is provided for C++ programmer
// convenience
//
/// No Error report
const Code OK = DAVIX_STATUS_OK;

/// Request executed partially
const Code PartialDone = DAVIX_STATUS_PARTIAL_DONE;

/// Error in the Webdav properties parsing
const Code WebDavPropertiesParsingError = DAVIX_STATUS_WEBDAV_PROPERTIES_PARSING_ERROR;

/// Wrong Uri, impossible to parse
const Code UriParsingError = DAVIX_STATUS_URI_PARSING_ERROR;

/// impossible to create a session
const Code SessionCreationError = DAVIX_STATUS_SESSION_CREATION_ERROR;

/// DNS resolution failure
const Code NameResolutionFailure= DAVIX_STATUS_NAME_RESOLUTION_FAILURE;

/// Impossible to connect, host down or network problem
const Code ConnectionProblem = DAVIX_STATUS_CONNECTION_PROBLEM;

/// redirection is needed manually
const Code RedirectionNeeded = DAVIX_STATUS_REDIRECTION_NEEDED;

/// Connexion timeout
const Code ConnectionTimeout= DAVIX_STATUS_CONNECTION_TIMEOUT;

/// operation timeout
const Code OperationTimeout= DAVIX_STATUS_OPERATION_TIMEOUT;

/// this operation is not supported
const Code OperationNonSupported= DAVIX_STATUS_OPERATION_NOT_SUPPORTED;

/// this file is not a directory
const Code IsNotADirectory = DAVIX_STATUS_IS_NOT_A_DIRECTORY;

/// Invalid file descriptor
const Code InvalidFileHandle = DAVIX_STATUS_INVALID_FILE_HANDLE;

/// Request already running
const Code AlreadyRunning = DAVIX_STATUS_ALREADY_RUNNING;

/// Authentication Error
const Code AuthentificationError = DAVIX_STATUS_AUTHENTIFICATION_ERROR;

/// Wrong Login and/or Password
const Code LoginPasswordError = DAVIX_STATUS_LOGIN_PASSWORD_ERROR;

/// Impossible to find specified credential
const Code CredentialNotFound = DAVIX_STATUS_CREDENTIAL_NOT_FOUND;

/// Permission deny
const Code PermissionRefused = DAVIX_STATUS_PERMISSION_REFUSED;

/// No such file, no such directoy, no such remote entity
const Code FileNotFound = DAVIX_STATUS_FILE_NOT_FOUND;

/// Action impossible, is a directory or a collection
const Code IsADirectory = DAVIX_STATUS_IS_A_DIRECTORY;

/// System related error
const Code SystemError = DAVIX_STATUS_SYSTEM_ERROR;

/// File already exist, impossible to create
const Code FileExist = DAVIX_STATUS_FILE_EXIST;

/// Invalid user argument
const Code InvalidArgument = DAVIX_STATUS_INVALID_ARG;

const Code InvalidServerResponse = DAVIX_STATUS_INVALID_SERVER_RESPONSE;

/// Unknow error
const Code UnknowError= DAVIX_STATUS_UNKNOW_ERROR;


}

///  @class DavixError
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
class DAVIX_EXPORT DavixError{
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
    /// set the scope of this error
    void setErrScope(const std::string & scope);

    ///
    /// get the scope of this error
    const std::string & getErrScope() const;


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
DAVIX_EXPORT std::string davix_scope_stat_str();
DAVIX_EXPORT std::string davix_scope_davOps_str();
DAVIX_EXPORT std::string davix_scope_mkdir_str();
DAVIX_EXPORT std::string davix_scope_directory_listing_str();
DAVIX_EXPORT std::string davix_scope_http_request();
DAVIX_EXPORT std::string davix_scope_xml_parser();
DAVIX_EXPORT std::string davix_scope_uri_parser();
DAVIX_EXPORT std::string davix_scope_io_buff();
DAVIX_EXPORT std::string davix_scope_x509cred();



//
DAVIX_EXPORT void davix_errno_to_davix_error(int errcode, const std::string & scope, const std::string & msg, DavixError** newErr);

} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
