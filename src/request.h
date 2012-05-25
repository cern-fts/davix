#ifndef DAVIX_REQUEST_H
#define DAVIX_REQUEST_H

#include <vector>
#include "libdavix_object.h"

namespace Davix {

class Request {
public:
    Request();

    /**
      Execute the given request and return result to the buffer result
      Execute the constructed query, throw an exception if an error occures
      @return 0 on success
      @throw Glib::Error : error string and protocol error code
     */
    virtual int execute_sync()  =0; // throw(Glib::Error)

    virtual const std::vector<char> & get_result()=0;



    /**
      return the current request code error
       ex : HTTP 200
     */
    virtual int get_request_code() =0;

};

} // namespace Davix

#endif // DAVIX_REQUEST_H
