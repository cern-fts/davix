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

    ssize_t simulateMultiPartRequest(const DavIOVecInput *input_vec,
                                     DavIOVecOuput * output_vec,
                       const dav_size_t count_vec, DavixError** tmp_err);
};


int find_header_params(char* buffer, dav_size_t* part_size, dav_off_t* part_offset);

int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err);

bool is_a_start_boundary_part(char* buffer, size_t s_buff, const std::string & boundary,
                            DavixError** err);

} // Davix

#endif // HTTPIOVEC_HPP
