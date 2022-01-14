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

#ifndef DAVIX_REQUESTPARAMS_HPP
#define DAVIX_REQUESTPARAMS_HPP

#include <vector>
#include <string>
#include <map>

#include "davix_request_params_types.hpp"
#include "../auth/davixauth.hpp"


/**
  @file davixrequestparams.hpp
  @author Devresse Adrien

  @brief C++ Davix configuration API
*/

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif


namespace Davix {

namespace gcloud {
    class Credentials;
}

struct RequestParamsInternal;



///
/// @class RequestParams
/// @brief Main container for Davix request options
///
/// RequestParams hold the davix request options :
/// authentication parameters, timeouts, user-agents,...
/// A Requestparams object can be shared between several Request
class DAVIX_EXPORT RequestParams
{
public:
    ///
    /// \brief default constructor
    ///
    RequestParams();
    ///
    /// \brief copy constructor
    /// \param params
    ///
    RequestParams(const RequestParams & params);
    ///
    /// \brief conveniencecopy constructor with NULL check
    /// \param params
    RequestParams(const RequestParams* params);
    /// \brief assignment operator
    RequestParams & operator=(const RequestParams & _p);

    virtual ~RequestParams();


    ///  disable the certificate authority validity check for the https request
    void setSSLCAcheck(bool chk);

    /// return the SSL Certificate authority validity check
    bool getSSLCACheck() const;

    /// set a X509 credential for a simple client authentication
    /// this function overwrite \ref setClientCertCallbackX509
    void setClientCertX509(const X509Credential & cli_cert);

    /// get the current client side credential
    const X509Credential &  getClientCertX509() const;

    /// set login/password for HTTP Authentication
    void setClientLoginPassword(const std::string & login, const std::string & password);

    /// get login/password for HTTP Authentication
    const std::pair<std::string,std::string> & getClientLoginPassword() const;


#ifdef __DAVIX_HAS_STD_FUNCTION
    /// set a function for or X509 client side dynamic authentication
    /// this function overwrite \ref setClientCertCallbackX509
    void setClientCertFunctionX509(const authFunctionClientCertX509 & callback);

    /// return the function
    const authFunctionClientCertX509 & getClientCertFunctionX509() const;
#endif

    /// set a callback for X509 client side dynamic authentication
    /// this function overwrite \ref setClientCertX509
    void setClientCertCallbackX509(authCallbackClientCertX509 callback, void* userdata);

    /// return the current client side callback for authentication with the associated user data
    std::pair<authCallbackClientCertX509,void*> getClientCertCallbackX509() const;

    /// set a callback for basic login/password http authentication
    /// this function overwrite \ref setClientLoginPassword
    void setClientLoginPasswordCallback(authCallbackLoginPasswordBasic callback, void* userdata);

    /// return the current login/password callback and the associated user data
    std::pair<authCallbackLoginPasswordBasic,void*> getClientLoginPasswordCallback() const;

    ///
    /// \brief define a Amazon S3 private key and public key
    /// \param secret_key secret key
    /// \param access_key public key
    ///
    void setAwsAuthorizationKeys(const AwsSecretKey & secret_key, const AwsAccessKey & access_key);

    ///
    /// \brief get Amazon S3 authentication tokens
    /// \return pair of secret key and public key
    ///
    const std::pair<AwsSecretKey, AwsAccessKey> & getAwsAutorizationKeys() const;

    ///
    /// \brief define a Amazon S3 bucket region
    /// \param region the region
    ///
    void setAwsRegion(const AwsRegion & region);

    ///
    /// \brief get Amazon S3 bucket region
    /// \return the bucket region
    ///
    const AwsRegion & getAwsRegion() const;

    ///
    /// \brief define an Amazon S3 security token
    /// \param token the security token
    ///
    void setAwsToken(const AwsToken & token);

    ///
    /// \brief get Amazon S3 security token
    /// \return the security token
    ///
    const AwsToken & getAwsToken() const;

    ///
    /// \brief set whether we're using an S3 path-based url
    /// \param alternate whether using an S3 path-based url
    ///
    void setAwsAlternate(const bool & alternate);

    ///
    /// \brief get whether we're using an S3 path-based url
    /// \return whether we're using an S3 path-based url
    ///
    const bool & getAwsAlternate() const;

    ///
    /// \brief set the secret key for Azure authentication
    /// \param key the secret key
    ///
    void setAzureKey(const AzureSecretKey & key);

    ///
    /// \brief get the secret key used for Azure authentication
    /// \return the secret key
    ///
    const AzureSecretKey & getAzureKey() const;

    ///
    /// \brief set the secret key for Azure authentication
    /// \param creds the secret key
    ///
    void setGcloudCredentials(const Davix::gcloud::Credentials & creds);

    ///
    /// \brief get the secret key used for Azure authentication
    /// \return the secret key
    ///
    const Davix::gcloud::Credentials & getGcloudCredentials() const;

    ///
    /// \brief set the OS token used for Swift authentication
    /// \param token the OS token
    ///
    void setOSToken(const OSToken & token);

    ///
    /// \brief get the OS token used for Swift authentication
    /// \return the OS token
    ///
    const OSToken & getOSToken() const;

    ///
    /// \brief set the OS project id used for Swift authentication
    /// \param id the project id
    ///
    void setOSProjectID(const OSProjectID & id);

    ///
    /// \brief get the OS project id used for Swift authentication
    /// \return the project id
    ///
    const OSProjectID & getOSProjectID() const;

    ///
    /// \brief set the Swift account used for Swift authentication
    /// \param account the Swift account
    ///
    void setSwiftAccount(const SwiftAccount & account);

    ///
    /// \brief get the Swift account used for Swift authentication
    /// \return the Swift account
    ///
    const SwiftAccount & getSwiftAccount() const;

    /// set listing mode flag for S3 bucket
    void setS3ListingMode(const S3ListingMode::S3ListingMode s3_listing_mode);

    /// get listing mode flag for S3 bucket
    S3ListingMode::S3ListingMode getS3ListingMode() const;

    /// set listing mode flag for Swift
    void setSwiftListingMode(const SwiftListingMode::SwiftListingMode swift_listing_mode);

    /// get listing mode flag for Swift
    SwiftListingMode::SwiftListingMode getSwiftListingMode() const;

    /// set maximum number of key entries return by S3 list object request
    void setS3MaxKey(const unsigned long s3_max_key_entries);

    /// get maximun number of key entries return by S3 list object request
    unsigned long getS3MaxKey() const;

    /// add the CA certificate in the directory 'path' as trusted certificate
    void addCertificateAuthorityPath(const std::string & path);

    /// get the list of the current user defined CA path
    const std::vector<std::string> & listCertificateAuthorityPath() const;

    /// define the connexion timeout
    /// conn_timeout is a relative time
    /// DEFAULT : 30s
    void setConnectionTimeout(struct timespec* conn_timeout);

    /// get the current connexion timeout
    const struct timespec * getConnectionTimeout()  const;


    /// define the maximum execution time for a davix request
    /// ops_timeout is a relative time
    /// DEFAULT : infinite
    void setOperationTimeout(struct timespec* ops_timeout);

    /// get the maximum execution time for a davix request
    /// DEFAULT : infinite
    const struct timespec * getOperationTimeout()const;


    /// enable or disable transparent redirection support
    /// In the transparent redirection mode,
    /// davix follows the HTTP redirection automatically
    /// DEFAULT : enabled
    void setTransparentRedirectionSupport(bool redirection);

    /// return true if the transparent redirection mode is enabled
    bool getTransparentRedirectionSupport() const;

    ///
    /// \brief number of re-try in case of operation failure
    /// \param number_retry
    ///
    /// define the number of retry attempt  in case of an operation failure
    void setOperationRetry(int number_retry);


    ///
    /// \brief getOperationRetry
    /// \return
    /// get current number of retry attempt, see \ref setOperationRetry for more details
    int getOperationRetry() const;


    ///
    /// \brief Delay in second between retry attempts
    ///// \param delay_retry
    ///
    /// define the number of seconds between retry attempts in case of slow servers
    void setOperationRetryDelay(int delay_retry);


    ///
    /// \brief getOperationRetryDelay
    /// \return
    /// get current number of seconds between retry attempts, see \ref setOperationRetryDelay for more details
    int getOperationRetryDelay() const;


    /// set copy mode for 3rd party copy
    void setCopyMode(const CopyMode::CopyMode copy_mode);

    /// get copy mode for 3rd party copy
    CopyMode::CopyMode getCopyMode() const;

    /// set recursive mode for directory operations
    void setRecursiveMode(const bool recursive_mode);

    /// get recursive mode for directory operations
    bool getRecursiveMode() const;

    /// set whether the server supports 100-continue. If enabled (default), 100-continue
    /// may or may not be sent to the server, depending on when davix decides it's
    /// appropriate. If disabled, 100-continue will never be used.
    void set100ContinueSupport(const bool enabled);

    /// get whether 100-continue support is enabled
    bool get100ContinueSupport() const;

#ifdef __DAVIX_HAS_STD_FUNCTION
    ///
    /// @brief setTransfertMonitorCb
    /// @param cb
    ///
    ///  define a transfer callback
    ///  The transfer callback is called on a regular based
    ///  when a data transfer operation is progressing ( put, get, copy )
    ///
    ///  The callback is called at least once per transfer for the following operations :
    ///  - DavFile::put
    ///  - DavixFile::get / getV
    ///  - Posix::read / pread / write / prwrite / preadVec
    ///
    ///
    void setTransfertMonitorCb(const TransferMonitorCB & cb);

    ///
    /// \brief getTransferMonitorCb
    /// \return current transfer monitor callback
    ///
    ///  see \ref setTransfertMonitorCb for more details
    ///
    const TransferMonitorCB & getTransferMonitorCb() const;

#endif //__DAVIX_HAS_STD_FUNCTION

    /// set the user agent for the associated request
    void setUserAgent(const std::string & user_agent);

    /// get the current user agent string
    const std::string & getUserAgent() const;

    /// set the request protocol ( ex : Webdav, Http-only, S3 )
    void setProtocol(const RequestProtocol::Protocol proto);

    /// get the current value of the request protocol
    RequestProtocol::Protocol getProtocol() const;

    /// Enable or disable the usage of the Metalink
    ///  (RFC-5854 and RFC-6249) with libdavix
    /// Metalink can be used for fail-over purpose, or multi-source download
    void setMetalinkMode( const MetalinkMode::MetalinkMode mode);

    /// get the Current Metalink mode
    MetalinkMode::MetalinkMode getMetalinkMode() const;

    /// set the keep alive value of the associated session
    void setKeepAlive(const bool keep_alive_flag);

    /// get the keep alive value of this request params
    bool getKeepAlive() const;


    /// Add a custom header line that has to be included in the requests
    ///  @param key key of the header
    ///  @param val value of the header
    void addHeader(const std::string &key, const std::string &val);

    /// return the list of custom headers configured
    const HeaderVec & getHeaders() const;

    /// set a SOCKS5 proxy server for intermediate usage
    /// example: setProxyServer("socks5://login:password@socks5.exmaple.org:8080")
    /// @param proxy_url url of the proxy server
    void setProxyServer(const Uri & proxy_url);

    /// get current SOCKS5 proxy server
    /// @return URL of the server or NULL if not defined
    const Uri* getProxyServer() const;

    /// internal usage
    void* getParmState() const;


    /// swap two RequestParams content
    /// fast operation
    void swap(RequestParams & params);

    /// get the number of retries that davix should perform in case
    /// it receives 202-Accepted on a GET request
    int getAcceptedRetry() const;

    /// set the number of retries that davix should perform in case
    /// it receives 202-Accepted on a GET request
    /// @param num_retries the number of retries
    void setAcceptedRetry(int num_retries);

    /// get the delay in seconds between retries that davix should
    /// perform in case it receives 202-Accepted on a GET request
    int getAcceptedRetryDelay() const;

    /// set the delay in seconds between retries that davix should
    /// perform in case it receives 202-Accepted on a GET request
    /// @param delay the delay in seconds
    void setAcceptedRetryDelay(int delay);
private:

   // dptr
    RequestParamsInternal* d_ptr;


};



} // namespace Davix

#endif // DAVIX_REQUESTPARAMS_HPP
