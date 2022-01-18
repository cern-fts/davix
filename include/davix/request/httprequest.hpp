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

#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <vector>
#include <unistd.h>
#include <utils/davix_types.hpp>
#include <utils/davix_uri.hpp>
#include <status/davixstatusrequest.hpp>
#include <params/davixrequestparams.hpp>

#ifndef __DAVIX_INSIDE__
#error "Only davix.h or davix.hpp should be included."
#endif

/**
  @file httprequest.hpp
  @author Devresse Adrien

  @brief Http low level request interface
 */




namespace Davix {

/// Callback for body providers
/// Before each time the body is provided, the callback will be called
/// once with buflen == 0.  The body may have to be provided >1 time
/// per request (for authentication retries etc.).
/// For a call with buflen > 0, the callback must return:
///    <0           : error, abort request; session error string must be set.
///     0           : ignore 'buffer' contents, end of body.
///     0 < x <= buflen : buffer contains x bytes of body data.  */
typedef dav_ssize_t (*HttpBodyProvider)(void *userdata,
                                    char *buffer, dav_size_t buflen);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

class NEONRequest;
class HttpCacheToken;
class ContentProvider;

namespace RequestFlag{
    ///
    /// \brief Request flag
    ///
    ///
    enum RequestFlag{
        SupportContinue100 = 0x01, /**< Enable support for 100 Continue code (default: OFF) */
        IdempotentRequest  = 0x02  /**< Specifie the request as Idempotent ( default : ON) */
    };

}

/// @class HttpRequest
/// @brief Http low level request interface.
///
/// HTTPRequest is the main davix class for low level HTTP queries.
///
/// HTTPRequest objects are provided by Davix::Context
///
class DAVIX_EXPORT HttpRequest : private NonCopyable
{
public:
    ///
    /// \brief HttpRequest constructor with a defined URL
    /// \param context davix context
    /// \param url URL of the resource
    /// \param err Davix error report system
    ///
    /// @snippet example_code_snippets.cpp HttpRequest uri
    HttpRequest(Context & context, const Uri & url, DavixError** err);

    ///
    /// \brief HttpRequest constructor with a defined URL from a string
    /// \param context davix context
    /// \param url URL of the resource
    /// \param err Davix error report system
    ///
    /// @snippet example_code_snippets.cpp HttpRequest
    HttpRequest(Context & context, const std::string & url, DavixError** err);

    ///
    /// \brief HttpRequest internal usage
    /// \param req
    ///
    HttpRequest(NEONRequest* req);
    virtual ~HttpRequest();

    ///  add a optional HTTP header request
    ///  replace an existing one if already exist
    ///  if the content of value of the header field is empty : remove an existing one
    ///  @param field  header field name
    ///  @param value header field value
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::addHeaderField
    void addHeaderField(const std::string & field, const std::string & value);

    ///
    /// \brief set the request method ( "GET", "PUT", ... )
    /// \param method request method
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::setRequestMethod
    void setRequestMethod(const std::string & method);


    ///
    /// \brief set the request parameter
    /// \param parameters Davix Request parameters
    ///
    ///  define the request parameters, can be used to define parameters
    ///  such as authentication scheme, timeout or user agent.
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::setParameters
    void setParameters(const RequestParams &parameters);

    ///   @brief execute this request completely
    ///
    ///   the answer is accessible with \ref Davix::HttpRequest::getAnswerContent
    ///   @param err davix error report
    ///   @return 0 on success
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::executeRequest
    int executeRequest(DavixError** err);

    ///
    ///  set the content of the request from a string
    ///  an empty string set no request content
    ///  @warning this string is not duplicated internally for performance reasons
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::setRequestBody
    void setRequestBody(const std::string & body);

    ///
    /// set the content of the request from a buffer
    ///  NULL pointer means a empty content
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::setRequestBody
    void setRequestBody(const void * buffer, dav_size_t len_buff);

    ///
    /// set the content of the request from a file descriptor
    /// start at offset and read a maximum of len bytes
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::setRequestBody
    void setRequestBody(int fd, dav_off_t offset, dav_size_t len);

    ///
    /// set a callback to provide the body of the requests
    ///
    void setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata);

    ///
    /// set a content provider object to provide the body of the requests
    ///
    void setRequestBody(ContentProvider &provider);

    ///
    /// @brief start a multi-part HTTP Request
    ///
    ///  the multi-part HTTP Request of davix
    ///  should be used for request with a large answer
    ///
    ///
    /// @param err : DavixError error report system
    /// @return return 0 if success, or a negative value if an error occures
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::beginRequest
    int beginRequest(DavixError** err);

    ///
    /// read a block of a maximum size bytes in the answer
    /// can return < max_size bytes depending of the data available
    ///
    /// @param buffer : buffer to fill
    /// @param max_size : maximum number of byte to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readBlock
    dav_ssize_t readBlock(char* buffer, dav_size_t max_size, DavixError** err);


    ///
    /// read a block of a maximum size bytes in the answer into buffer
    /// can return < max_size bytes depending of the data available
    ///
    /// @param buffer : vector to fill
    /// @param max_size : maximum number of byte to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readBlock
    dav_ssize_t readBlock(std::vector<char> & buffer, dav_size_t max_size, DavixError** err);

    ///
    /// read a segment of size bytes, return always max_size excepted if the end of the content is reached
    ///
    /// @param buffer : vector to fill
    /// @param max_size : maximum number of byte to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readSegment
    dav_ssize_t readSegment(char* buffer, dav_size_t max_size, DavixError** err);



    ///
    /// write the full answer content to the given file descriptor
    /// @param fd : buffer to fill
    /// @param err : DavixError error report system
    /// @return number of bytes read
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readToFd
    dav_ssize_t readToFd(int fd, DavixError** err);

    ///
    /// write the first 'read_size' first bytes to the given file descriptor
    /// @param fd : buffer to fill
    /// @param read_size : number of bytes to read
    /// @param err : DavixError error report system
    /// @return number of bytes read
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readToFd
    dav_ssize_t readToFd(int fd, dav_size_t read_size, DavixError** err);

    ///
    /// read a line of text of a maximum size bytes in the answer
    /// @param buffer : buffer to fill
    /// @param max_size : maximum number of bytes to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed, if return == max_size -> the line too big
    ///
    ///  @snippet example_code_snippets.cpp HttpRequest::readLine
    dav_ssize_t readLine(char* buffer, dav_size_t max_size, DavixError** err);

    ///
    /// discard the response body
    /// @param err: DavixError error report system
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::discardBody
    void discardBody(DavixError** err);

    ///
    /// finish a request stated with beginRequest
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::endRequest
    int endRequest(DavixError** err);

    /// return the body of the answer
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getAnswerContent
    const char* getAnswerContent();

    /// return the body of the answer in a vector
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getAnswerContentVec
    std::vector<char>  & getAnswerContentVec();

    /// get content length
    /// @return content size, return -1 if chunked
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getAnswerSize
    dav_ssize_t getAnswerSize() const;

    /// get last modified time
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getLastModified
    time_t getLastModified() const;

    ///
    ///  clear the current result
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::clearAnswerContent
    void clearAnswerContent();

    ///
    /// @return current request code error
    /// undefined if executeRequest or beginRequest has not be called before
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getRequestCode
    int getRequestCode();

    ///
    /// get the value associated to a header key in the request answer
    /// @param header_name : key of the header field
    /// @param value : reference of the string to set
    /// @return true if this header exist or false if it does not
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getAnswerHeader
    bool getAnswerHeader(const std::string & header_name, std::string & value) const;


    ///
    /// get all the headers associated with this answer
    /// @param vec_headers : vector of headers
    /// @return true if this header exist or false if it does not
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::getAnswerHeaders
    size_t getAnswerHeaders( HeaderVec & vec_headers) const;


    /// @deprecated not in use anymore
    DEPRECATED(HttpCacheToken* extractCacheToken() const);

    /// @deprecated not in use anymore
    DEPRECATED(void useCacheToken(const HttpCacheToken* token));

    /// set a HttpRequest flag
    void setFlag(const RequestFlag::RequestFlag flag, bool value);

    /// get a HttpRequest flag value
    bool getFlag(const RequestFlag::RequestFlag flag);

private:
    NEONRequest* d_ptr;

    void runPreRunHook();
    void runRegisterHooks();
};


/// @class GetRequest
/// @brief Http low level request configured for GET operation
class GetRequest : public HttpRequest{
public:
    ///
    /// \brief Construct a HttpRequest for GET a operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::GetRequest
    GetRequest(Context & context, const Uri & uri, DavixError** err);
};

/// @class PutRequest
/// @brief Http low level request configured for PUT operation
class PutRequest : public HttpRequest{
public:
    ///
    /// \brief Construct a HttpRequest for PUT a operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::PutRequest
    PutRequest(Context & context, const Uri & uri, DavixError** err);
};

/// @class PostRequest
/// @brief Http low level request configured for POST operation
class PostRequest : public HttpRequest{
public:
    ///
    /// \brief Construct a HttpRequest for POST a operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::PutRequest
    PostRequest(Context & context, const Uri & uri, DavixError** err);
};


/// @class HeadRequest
/// @brief Http low level request configured for HEAD operation
class HeadRequest : public HttpRequest{
public:
    ///
    /// \brief Construct a HttpRequest for a HEAD operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::HeadRequest
    HeadRequest(Context & context, const Uri & uri, DavixError** err);
};


/// @class DeleteRequest
/// @brief Http low level request configured for DELETE operation
class DeleteRequest : public HttpRequest{
public:
    ///
    /// \brief  Construct a HttpRequest forfor a DELETE operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::DeleteRequest
    DeleteRequest(Context & context, const Uri & uri, DavixError** err);
};


/// @class PropfindRequest
/// @brief Webdav low level request configured for PROPFIND operation
class PropfindRequest : public HttpRequest{
public:
    ///
    /// \brief  Construct a HttpRequest for a PROPFIND operation
    /// \param context
    /// \param uri
    /// \param err
    ///
    /// @snippet example_code_snippets.cpp HttpRequest::PropfindRequest
    PropfindRequest(Context & context, const Uri & uri, DavixError** err);
};

}

#endif // DAVIX_HTTPREQUEST_H
