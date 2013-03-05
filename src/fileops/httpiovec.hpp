#ifndef HTTPIOVEC_HPP
#define HTTPIOVEC_HPP

#include <davix.hpp>
#include <fileops/iobuffmap.hpp>

namespace Davix{



class HttpVecOps
{
public:
    HttpVecOps(HttpRequest & req);

    ssize_t readPartialBufferVec(const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err);

private:
    HttpRequest & _req;


    ssize_t parseMultipartRequest(const DavIOVecInput *input_vec,
                                                DavIOVecOuput * output_vec,
                                  const dav_size_t count_vec, DavixError** tmp_err);
};


int find_header_params(char* buffer, dav_size_t* part_size, dav_off_t* part_offset);

} // Davix

#endif // HTTPIOVEC_HPP
