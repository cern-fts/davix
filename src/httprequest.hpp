#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <vector>

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

class NEONRequest;
class NEONSessionFactory;

///
/// @class HTTPRequest
/// HTTPRequest is the main davix class for low level HTTP queries
/// HTTPRequest objects are provided by Davix::Context
///
class HttpRequest
{
public:
    HttpRequest(NEONRequest* req);
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
    void setRequestBodyString(const std::string & body);

    ///
    /// set the content of the request from a buffer
    ///  NULL pointer means a empty content
    ///
    void setRequestBodyBuffer(const void * buffer, size_t len_buff);

    ///
    /// set the content of the request from a file descriptor
    /// start at offset and read a maximum of len bytes
    ///
    void setRequestBodyFileDescriptor(int fd, off_t offset, size_t len);

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
    /// read a block of a maximum size bytes in the request
    /// @param buffer : buffer to fill
    ///  @param max_size : maximum number of byte to read
    ///  @return number of bytes readed
    ///
    ssize_t readBlock(char* buffer, size_t max_size, DavixError** err);


    ///
    /// finish a request stated with beginRequest
    int endRequest(DavixError** err);

    /// get a reference to the internal anwser content buffer
    const std::vector<char> & getAnswerContent();

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
    bool getAnswerHeader(const std::string & header_name, std::string & value);




private:
    HttpRequest(const HttpRequest &req);
    HttpRequest & operator=(const HttpRequest &);

    NEONRequest* d_ptr;

    friend class NEONRequest;
    friend class NEONSessionFactory;

};




///
/// return true if this http code is a success
///
bool httpcodeIsValid(int code);

void httpcodeToDavixCode(int code, const std::string & scope, const std::string & end_message, DavixError** err);


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
