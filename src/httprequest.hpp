#ifndef DAVIX_HTTPREQUEST_H
#define DAVIX_HTTPREQUEST_H

#include <vector>

#include <status/davixstatusrequest.hpp>
#include <davixrequestparams.hpp>



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

    void set_parameters(const RequestParams &p );
    /**
      Execute the given request and return result to the buffer result
      Execute the constructed query, throw an exception if an error occures
      @return 0 on success
     */
    int executeRequest(DavixError** err);

    /**
        Define a buffer for the full request body content
     */
    void add_full_request_content(const std::string & body);

    int execute_block(DavixError** err);
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of byte to read
      @return number of bytes readed
    */
    ssize_t read_block(char* buffer, size_t max_size, DavixError** err);
    /**
      finish an already started request
     */
    int finish_block(DavixError** err);
    /**
      get a reference to the current result
     */
    const std::vector<char> & get_result();
    /**
      clear the current result
    */
    void clear_result();



    /**
      return the current request code error
       ex : HTTP 200
     */
    virtual int getRequestCode();


private:
    NEONRequest* d_ptr;

    friend class NEONRequest;
    friend class NEONSessionFactory;

};


/**
Translate  http code error to errno code
 */
int httpcode_to_errno(int code);

///
/// return true if this http code is a success
///
bool httpcodeIsValid(int code);

void httpcodeToDavixCode(int code, const std::string & scope, const std::string & end_message, DavixError** err);


} // namespace Davix

#endif // DAVIX_HTTPREQUEST_H
