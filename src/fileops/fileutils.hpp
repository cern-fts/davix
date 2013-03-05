#ifndef DAVIX_FILEUTILS_HPP
#define DAVIX_FILEUTILS_HPP

#include <davix.hpp>

namespace Davix {

extern const std::string ans_header_byte_range;
extern const std::string ans_header_content_type;
extern const std::string ans_header_multi_part_value;
extern const std::string ans_header_boundary_field;
extern const std::string ans_header_content_length;


// take a HTTP request status and convert file status to common errcode
int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err);


// configure Range request
void setup_offset_request(HttpRequest* req, const off_t *start_len, const size_t *size_read, const size_t number_ops);

} // namespace Davix

#endif // DAVIX_FILEUTILS_HPP
