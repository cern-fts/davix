/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#ifndef REQUEST_PARAMS_TYPES_HPP
#define REQUEST_PARAMS_TYPES_HPP

/**
  @file davix_request_params_types.hpp
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
        AwsS3,
        // Use Microsoft Azure API
        Azure,
        // Use gcloud API
        Gcloud,
        // Use Swift API
        Swift,
        // Use CS3 API
        CS3
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

namespace S3ListingMode{
    enum S3ListingMode{
        // Full hierarchical listing (depth is 1)
        Hierarchical,
        // Semi-hierarchical listing (depth is infinity)
        SemiHierarchical,
        // Flat listing
        Flat
    };
}

namespace SwiftListingMode{
    enum SwiftListingMode{
        // Full hierarchical listing (depth is 1)
        Hierarchical,
        // Semi-hierarchical listing (depth is infinity)
        SemiHierarchical
    };
}

namespace CopyMode{
    enum CopyMode{
        // source push to destination
        Push,
        // destination pull from source
        Pull
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
///
/// \brief string for Amazon region
///
typedef std::string AwsRegion;
///
/// \brief string for Amazon security token
///
typedef std::string AwsToken;

///
/// \brief string for Azure private key
///
typedef std::string AzureSecretKey;

///
/// \brief string for Openstack token
///
typedef std::string OSToken;

///
/// \brief string for Openstack project ID
///
typedef std::string OSProjectID;

///
/// \brief string for Swift Account
///
typedef  std::string SwiftAccount;

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
