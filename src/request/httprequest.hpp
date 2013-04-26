#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <vector>
#include <unistd.h>
#include <davix_types.h>
#include <davixuri.hpp>
#include <status/davixstatusrequest.hpp>
#include <params/davixrequestparams.hpp>
#include <request/httpcachetoken.hpp>

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
class NEONSessionFactory;

namespace RequestFlag{
    enum RequestFlag{
        SupportContinue100 = 0x01, /**< Enable support for 100 Continue code (default: OFF) */
        IdempotentRequest  = 0x02  /**< Specifie the request as Idempotent ( default : ON) */
    };

}


/// @class HTTPRequest
/// @brief Http low level request interface
/// HTTPRequest is the main davix class for low level HTTP queries
/// HTTPRequest objects are provided by Davix::Context
///
class DAVIX_EXPORT HttpRequest
{
public:
    HttpRequest(NEONRequest* req);
    HttpRequest(Context & context, const Uri & uri, DavixError** err);
    HttpRequest(Context & context, const std::string & url, DavixError** err);
    virtual ~HttpRequest();

    ///  add a optional HTTP header request
    ///  replace an existing one if already exist
    ///  if the content of value of the header field is empty : remove an existing one
    ///  @param field :  header field name
    ///  @param value :  header field value
    void addHeaderField(const std::string & field, const std::string & value);

    ///
    /// set the request command to execute ( GET, POST, etc... )
    /// DEFAULT : GET
    void setRequestMethod(const std::string & request_str);


    ///
    /// define HttpRequest parameter
    /// see \ref RequestParams for more details
    /// a copy of the object is done
    ///
    void setParameters(const RequestParams &p );

    ///   @brief execute this request completely
    ///
    ///   the answer is accessible with \ref Davix::HttpRequest::getAnswerContent
    ///   @param err : davix error report
    ///   @return 0 on success
    int executeRequest(DavixError** err);

    ///
    ///  set the content of the request from a string
    ///  an empty string set no request content
    ///  @warning this string is not duplicated internally for performance reasons
    ///
    void setRequestBody(const std::string & body);

    ///
    /// set the content of the request from a buffer
    ///  NULL pointer means a empty content
    ///
    void setRequestBody(const void * buffer, dav_size_t len_buff);

    ///
    /// set the content of the request from a file descriptor
    /// start at offset and read a maximum of len bytes
    ///
    void setRequestBody(int fd, dav_off_t offset, dav_size_t len);

    ///
    /// set a callback to provide the body of the requests
    ///
    void setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata);

    ///
    /// @brief start a multi-part HTTP Request
    ///
    ///  the multi-part HTTP Request of davix
    ///  should be used for request with a large answer
    ///
    ///
    ///  example :
    ///   DavixError* tmp_err=NULL;
    ///   beginRequest(&tmp_err) //
    ///   do{
    ///        ret= readBlock(buffer, size_read, &tmp_err);
    ///   } while(ret > 0);
    ///   endRequest(&tmp_err);
    ///
    /// @param err : DavixError error report system
    /// @return return 0 if success, or a negative value if an error occures
    ///
    int beginRequest(DavixError** err);

    ///
    /// read a block of a maximum size bytes in the answer
    /// @param buffer : buffer to fill
    /// @param max_size : maximum number of byte to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed
    ///
    dav_ssize_t readBlock(char* buffer, dav_size_t max_size, DavixError** err);


    ///
    /// read a block of a maximum size bytes in the answer into buffer
    /// @param buffer : vector to fill
    /// @param max_size : maximum number of byte to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed
    ///
    dav_ssize_t readBlock(std::vector<char> & buffer, dav_size_t max_size, DavixError** err);


    ///
    /// write the full answer content to the given file descriptor
    /// @param fd : buffer to fill
    /// @param err : DavixError error report system
    /// @return number of bytes read
    ///
    dav_ssize_t readToFd(int fd, DavixError** err);

    ///
    /// write the first 'read_size' first bytes to the given file descriptor
    /// @param fd : buffer to fill
    /// @param read_size : number of bytes to read
    /// @param err : DavixError error report system
    /// @return number of bytes read
    ///
    dav_ssize_t readToFd(int fd, dav_size_t read_size, DavixError** err);

    ///
    /// read a line of text of a maximum size bytes in the answer
    /// @param buffer : buffer to fill
    /// @param max_size : maximum number of bytes to read
    /// @param err : DavixError error report system
    /// @return number of bytes readed, if return == max_size -> the line too big
    ///
    dav_ssize_t readLine(char* buffer, dav_size_t max_size, DavixError** err);

    ///
    /// finish a request stated with beginRequest
    int endRequest(DavixError** err);

    /// get a reference to the internal anwser content buffer
    const char* getAnswerContent();

    /// get content length
    dav_size_t getAnswerSize() const;

    ///
    ///  clear the current result
    ///
    void clearAnswerContent();

    ///
    /// @return current request code error
    /// undefined if executeRequest or beginRequest has not be called before
    int getRequestCode();

    ///
    /// get the value associated to a header key in the request answer
    /// @param header_name : key of the header field
    /// @param value : reference of the string to set
    /// @return true if this header exist or false if it does not
    bool getAnswerHeader(const std::string & header_name, std::string & value) const;

    ///
    /// @brief Extract a cache token from this Request
    ///
    /// a cache token allows to optimize future request
    /// cache tokens are dynamically allocated and need to be free
    /// @return cache token or null pointer if not available
    HttpCacheToken* extractCacheToken() const;

    /// set a cache token associated with this request
    ///
    void useCacheToken(const HttpCacheToken* token);

    /// set a HttpRequest flag
    void setFlag(const RequestFlag::RequestFlag flag, bool value);

    /// get a HttpRequest flag value
    bool getFlag(const RequestFlag::RequestFlag flag);

private:
    HttpRequest(const HttpRequest &req);
    HttpRequest & operator=(const HttpRequest &);

    NEONRequest* d_ptr;

    friend class NEONRequest;
    friend class NEONSessionFactory;

};


/// @class GetRequest
/// @brief Http low level request configured for GET operation
class GetRequest : public HttpRequest{
public:
    GetRequest(Context & context, const Uri & uri, DavixError** err);
};

/// @class PutRequest
/// @brief Http low level request configured for PUT operation
class PutRequest : public HttpRequest{
public:
    PutRequest(Context & context, const Uri & uri, DavixError** err);
};

/// @class HeadRequest
/// @brief Http low level request configured for HEAD operation
class HeadRequest : public HttpRequest{
public:
    HeadRequest(Context & context, const Uri & uri, DavixError** err);
};


/// @class DeleteRequest
/// @brief Http low level request configured for DELETE operation
class DeleteRequest : public HttpRequest{
public:
    DeleteRequest(Context & context, const Uri & uri, DavixError** err);
};


/// @class PropfindRequest
/// @brief Webdav low level request configured for PROPFIND operation
class PropfindRequest : public HttpRequest{
public:
    PropfindRequest(Context & context, const Uri & uri, DavixError** err);
};



DAVIX_EXPORT bool httpcodeIsValid(int code);

DAVIX_EXPORT void httpcodeToDavixCode(int code, const std::string & scope, const std::string & end_message, DavixError** err);

}

#endif // DAVIX_HTTPREQUEST_H
