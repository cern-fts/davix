#ifndef DAVIX_DAVIXSTATUSREQUEST_HPP
#define DAVIX_DAVIXSTATUSREQUEST_HPP

#include <string>
#include <davix_types.h>
#include <iostream>

/**
  @file davixstatusrequest.hpp
  @author Devresse Adrien, CERN

  @brief Error report system of davix
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix {

class Context;
class NGQRequest;
struct DavixErrorInternal;

namespace StatusCode{



/// Common Error code of Davix
/// See \ref DavixError for more details
///
enum Code {
    /// No Error report
    OK = 0x000,

    /// Request executed partially
    PartialDone = 0x001,

    /// Error in the Webdav properties parsing
    WebDavPropertiesParsingError = 0x002,

    /// Error in the Webdav properties parsing
    UriParsingError = 0x003,

    /// impossible to create a session
    SessionCreationError = 0x004,

    /// DNS resolution failure
    NameResolutionFailure= 0x005,

    /// Impossible to connect, host down or network problem
    ConnectionProblem = 0x006,

    /// redirection is needed manually
    RedirectionNeeded = 0x007,

    /// Impossible to connect, host down or network problem
    ConnectionTimeout = 0x008,

    /// operation timeout
    OperationTimeout = 0x009,

    /// this operation is not supported
    OperationNonSupported= 0x00a,

    /// Action impossible, is a directory or a collection
    IsNotADirectory = 0x00b,

    /// Invalid file descriptor
    InvalidFileHandle = 0x00c,

    /// Request already running
    AlreadyRunning = 0x00d,

    /// Authentication Error
    AuthenticationError= 0x00e,

    /// Wrong Login and/or Password
    LoginPasswordError = 0x00f,

    /// Impossible to find specified credential
    CredentialNotFound = 0x010,

    /// Permission deny, Authorisation problem ( EACCESS, EPERM )
    PermissionRefused = 0x011,

    /// File not found (ENOENT )
    FileNotFound = 0x012,

    /// This file is not a regular file but a directory ( EISDIR )
    IsADirectory = 0x013,

    /// System call related error
    SystemError = 0x014,

    /// File already exist ( EEXIST )
    FileExist = 0x015,

    /// Invalid argument from user ( EINVAL )
    InvalidArgument = 0x016,

    /// Server answer problem ( > 500 )
    InvalidServerResponse = 0x017,

    /// SSL/TLS layer Error
    SSLError = 0x018,

    /// Impossible to decrypt client credential for usage
    CredDecryptionError = 0x019,
    
    /// Operation canceled
    Canceled = 0x020,
    
    /// Delegation error
    DelegationError = 0x021,
    
    /// Remote error. Used for third party copies: it means the
    /// destination failed.
    RemoteError = 0x022,

    /// Generic Parsing Error
    ParsingError = 0x23,

    /// Undefined error
    UnknowError = 0x100


};

}

///  @class DavixError
///  @brief Davix Error Handler
///
/// Error report system of Davix, similar behavior to the Glib Error report
/// Davix does not use C++ exception
///
///
/// Each function which takes a DavixError** as argument can take the value NULL
///
/// Example :
///
///
class DAVIX_EXPORT DavixError{
public:

    ///
    /// Construct a DavixError object
    ///
    /// @param scope : string parameter representing the scope of the error
    /// @param errCode : Davix Error code, see  Davix::StatusCode::Code
    /// @param errMsg : String representation of the error
    DavixError(const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);
    ///
    /// \brief copy constructor
    /// \param e
    ///
    DavixError(const DavixError & e);
    ///
    /// \brief assignment operator
    /// \param e
    /// \return
    ///
    DavixError & operator=(const DavixError & e);
    ///
    /// \brief ~DavixError
    ///
    virtual ~DavixError();


    ///
    /// \brief clone Error
    /// \return new dynamically allocated copy of the Error
    ///
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
    /// \brief create a new DavixError
    /// \param err pointer to a DavixError pointer
    /// \param scope scope of the Error
    /// \param errCode Error code
    /// \param errMsg Error message
    ///
    ///
    /// create a new dynamically allocated DavixError Object
    /// if err is NULL, silent suppress the error report
    static void setupError(DavixError** err, const std::string & scope, StatusCode::Code errCode, const std::string & errMsg);

    ///
    /// clear the content of the current error and set err to NULL
    static void clearError(DavixError** err);



    ///
    /// \brief propagate an Error structure to an upper level
    /// \param newErr
    /// \param oldErr
    ///
    ///  propagate the Davix Error Object from oldErr to newErr
    ///  OldErr can be consider as free after this operation
    ///  erase the current error if newErr is not NULL
    ///
    static void propagateError(DavixError** newErr, DavixError* oldErr);




    ///
    /// \brief propagatePrefixedError
    /// \param newErr
    /// \param oldErr
    /// \param prefix
    ///
    /// same than propagateError but add a string prefix in front of the error description
    ///
    //////
    static void propagatePrefixedError(DavixError** newErr, DavixError* oldErr, const std::string & prefix);

private:
   DavixErrorInternal * d_ptr;
};


/// \cond PRIVATE_SYMBOLS
///
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

// !!!!!!!!!!!!!!!!!!!
// Warning: Deprecated symbols, do not use anymore
// /////////////////////////////////////////////////
// Deprecated, do not use, API compability only
namespace StatusCode{
const Code AuthentificationError = AuthenticationError;
}

typedef enum StatusCode::Code davix_status_t;


/// \endcond PRIVATE_SYMBOLS

} // namespace Davix

#endif // DAVIX_DAVIXSTATUSREQUEST_H
