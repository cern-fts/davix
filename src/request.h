#ifndef DAVIX_REQUEST_H
#define DAVIX_REQUEST_H

#include <vector>
#include "libdavix_object.h"

namespace Davix {

#define BUFFER_SIZE 16000

class Request {
public:
    Request();
    virtual ~Request(){}

    /**
      Execute the given request and return result to the buffer result
      Execute the constructed query, throw an exception if an error occures
      @return 0 on success
      @throw Glib::Error : error string and protocol error code
     */
    virtual int execute_sync()=0; // throw(Glib::Error)


    virtual void execute_block()=0;
    /**
      read a block of a maximum size bytes in the request
      @param buffer : buffer to fill
      @param max_size : maximum number of bytes to set
      @throw Glib::Error
    */
    virtual ssize_t read_block(char* buffer, size_t max_size)=0;
    /**
      finish an already started request
     */
    virtual void finish_block()=0;
    /**
      get a reference to the current result
     */
    virtual const std::vector<char> & get_result()=0;
    /**
      clear the current result
    */
    virtual void clear_result()=0;



    /**
      return the current request code error
       ex : HTTP 200
     */
    virtual int get_request_code() =0;

};

} // namespace Davix

#endif // DAVIX_REQUEST_H
