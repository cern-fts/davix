#ifndef DAVIX_FILEUTILS_HPP
#define DAVIX_FILEUTILS_HPP

#include <davix.hpp>
#include <fileproperties.hpp>
#include <boost/function.hpp>

namespace Davix {

extern const std::string ans_header_byte_range;
extern const std::string ans_header_content_type;
extern const std::string ans_header_multi_part_value;
extern const std::string ans_header_boundary_field;
extern const std::string ans_header_content_length;
extern const std::string req_header_byte_range;


// take a HTTP request status and convert file status to common errcode
int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err);


// configure Range request
void setup_offset_request(HttpRequest* req, const dav_off_t *start_len, const dav_size_t *size_read, const dav_size_t number_ops);

void fill_stat_from_fileproperties(struct stat* st, const  FileProperties & prop);



typedef boost::function<int (dav_off_t &, dav_off_t &)> OffsetCallback;

std::vector< std::pair<dav_size_t, std::string> > generateRangeHeaders(dav_size_t max_header_size, OffsetCallback & offset_provider);


} // namespace Davix

#endif // DAVIX_FILEUTILS_HPP
