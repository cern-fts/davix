#ifndef REQUEST_PARAMS_TYPES_HPP
#define REQUEST_PARAMS_TYPES_HPP

/**
  @file request_params_types.hpp
  @author Devresse Adrien

  @brief types for request parameters
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

#include <functional>

#include "../utils/davix_uri.hpp"


namespace Davix{

namespace RequestProtocol{
    ///
    /// \brief Http based protocol to use for advanced queries
    ///
    enum Protocol{
        // default
        Auto=0,
        // Strict Http without extensions
        Http,
        // Use Http + Webdav extension
        Webdav,
        // Use Amazon S3 API
        AwsS3
    };
}

namespace MetalinkMode{
    enum MetalinkMode{
        // default mode = failover
        Auto=0,
        // Disable Metalink Support
        Disable,
        // Use only for failover purpose
        FailOver,
        // Enable parallel DL
        XStream
    };
}


///
/// \brief string for Amazon private key
///
typedef std::string AwsSecretKey;
///
/// \brief string for Amazon public key
///
typedef std::string AwsAccessKey;


#ifdef __DAVIX_HAS_STD_FUNCTION

    ///
    /// \brief DataProviderFun
    ///
    /// Before each time the body is provided, the callback will be called
    /// once with max_size == 0.  The body may have to be provided >1 time
    /// per request (for authentication retries etc.).
    ///
    /// For a call with max_size == 0, the callback must return zero on success
    /// or non-zero on error
    ///
    /// For a call with max_size > 0, the callback must return:
    ///       <0           : error, abort request
    ///        0           : ignore 'buffer' contents, end of data.
    ///     0 < x <= buflen : buffer contains x bytes of data.  */
    ///
    typedef std::function<dav_ssize_t (void* buffer, dav_size_t max_size)> DataProviderFun;



    namespace Transfer{
        enum Type{
            Read =0x00,
            Write =0x01,

        };

    }

    ///
    /// \brief TransferMonitorCB
    ///
    ///
    ///  Monitor the number of bytes transfered for an associated query
    ///
    ///  url : url of the resource
    ///  baudrate: baudrate for the transfer in bytes/second
    ///  bytes_transfered: bytes transfered at the time of the callback called
    ///  total_size: total size of the resource if available, can be 0 if not determined
    ///
    typedef std::function<void (const Uri & url, Transfer::Type type, dav_ssize_t bytes_transfered, dav_size_t total_size)> TransferMonitorCB;

#endif




} // Davix


#endif // REQUEST_PARAMS_TYPES_HPP
