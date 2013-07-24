#ifndef HTTPIOVEC_HPP
#define HTTPIOVEC_HPP

#include <davix.hpp>
#include <fileops/iobuffmap.hpp>

namespace Davix{

// simple chunk info handler
struct ChunkInfo {
    ChunkInfo() :
        offset(0),
        size(0),
        bounded(false){}
    dav_off_t offset;
    dav_size_t size;
    bool bounded;
};


class HttpVecOps
{
public:
    HttpVecOps(Context & c, const Uri & u, const RequestParams & params,
               const HttpCacheToken & cacheToken) :
        _c(c),
        _url(u),
        _params(params),
        _cacheToken(cacheToken)
    {}

    dav_ssize_t readPartialBufferVec(const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err);

private:

    dav_ssize_t readPartialBufferVecRequest(HttpRequest & req,
                              const DavIOVecInput * input_vec,
                              DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err);

    dav_ssize_t parseMultipartRequest(HttpRequest & req,
                                      const DavIOVecInput *input_vec,
                                      DavIOVecOuput * output_vec,
                                      const dav_size_t count_vec, DavixError** tmp_err);

    dav_ssize_t simulateMultiPartRequest(HttpRequest & req,
                                         const DavIOVecInput *input_vec,
                                         DavIOVecOuput * output_vec,
                                         const dav_size_t count_vec, DavixError** tmp_err);
private:
    Context &_c;
    const Uri &_url;
    const RequestParams &_params;
    const HttpCacheToken &_cacheToken;
};



bool is_a_start_boundary_part(char* buffer, dav_size_t s_buff, const std::string & boundary,
                            DavixError** err);

int find_header_params(char* buffer, dav_size_t buffer_len, dav_size_t* part_size, dav_off_t* part_offset);

int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err);


int parse_multi_part_header(HttpRequest& req, const std::string & boundary, ChunkInfo & info,
                            int & n_try, DavixError** err);

int davIOVecProvider(const DavIOVecInput *input_vec, dav_ssize_t & counter, dav_ssize_t number, dav_off_t & begin, dav_off_t & end);




void HttpIoVecSetupErrorMultiPart(DavixError** err);

} // Davix

#endif // HTTPIOVEC_HPP
