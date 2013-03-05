#include <config.h>
#include "httpiovec.hpp"
#include <cstring>
#include <logger/davix_logger_internal.h>
#include <deque>
#include <functional>

namespace Davix{

const std::string HttpIoVec_scope(){
    return "Davix::HttpVecOps";
}

typedef std::pair< const DavIOVecInput *, DavIOVecOuput *> PartPtr;
typedef std::deque<PartPtr> PartPtrDeck;


HttpVecOps::HttpVecOps(HttpRequest &req) :
    _req(req)
{

}

void configure_iovec_range_header(HttpRequest& req, const DavIOVecInput * input_vec, const dav_size_t count_vec){
    off_t offset_tab[count_vec];
    size_t size_tab[count_vec];

    for(dav_size_t i =0; i < count_vec; ++i){
        offset_tab[i] = input_vec[i].diov_offset;
        size_tab[i] = input_vec[i].diov_size;
    }
    setup_offset_request(&req, offset_tab, size_tab, count_vec);
}



ssize_t HttpVecOps::readPartialBufferVec(const DavIOVecInput * input_vec,
                          DavIOVecOuput * output_vec,
                          const dav_size_t count_vec, DavixError** err){
       ssize_t ret;
       DavixError* tmp_err=NULL;
       configure_iovec_range_header(_req, input_vec, count_vec);
       if( _req.beginRequest(&tmp_err) >0){
           const int retcode = _req.getRequestCode();
           switch(retcode){
                case 206: // multipart req
                    ret = parseMultipartRequest( input_vec,
                                             output_vec, count_vec, &tmp_err);
                    break;
                case 200: // classical req, simulate vector ops
                    // TODO
                    ret = -1;
                    break;
                default:
                    httpcodeToDavixCode(_req.getRequestCode(),davix_scope_http_request(),", ", &tmp_err);
                    ret = -1;
           }
       }
       if(tmp_err)
           DavixError::propagateError(err, tmp_err);
       return ret;
}

void HttpIoVecSetupErrorMultiPart(DavixError** err){
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, "Invalid Multi-Part HTTP response");
}

inline bool isValidBoundaryChar(const char* p){
    return ( *p != '\0' && *p != '"' && *p != '\t' && *p != '\n');
}



int check_multi_part_content_type(const HttpRequest& req, std::string & boudary, DavixError** err){

    size_t pos_bound;
    std::string buffer;

    if( req.getAnswerHeader(ans_header_content_range, buffer) == true // has content type
           && ( strncmp(buffer.c_str(), ans_header_multi_part_value.c_str(), ans_header_multi_part_value.size()) == 0) // has multipart
           && ( ( pos_bound= buffer.find(ans_header_boundary_field)) != std::string::npos)){
          char* p, *origin_p;
          origin_p = p = (char*) buffer.c_str() + pos_bound;
          if(*p == '"')
              ++p;
          while( isValidBoundaryChar(p))
              ++p;
          if( p != origin_p ){
              DAVIX_TRACE("Multi part content type : %s", boudary.c_str());
              boudary.assign(buffer.c_str(), p - buffer.c_str());
              return 0;
          }
    }
    HttpIoVecSetupErrorMultiPart(err);
    return -1;
}


bool is_a_start_boundary_part(char* buffer, size_t s_buff, const std::string & boundary,
                            DavixError** err){
    if(s_buff > 3){
        char * p = buffer;
        if( *p == '-' && *(p+1)== '-'){
            if( strcmp(buffer, boundary.c_str()) ==0){
                return true;
            }
        }
    }
    DAVIX_TRACE("Invalid boundary delimitation");
    HttpIoVecSetupErrorMultiPart(err);
    return false;
}

inline char* header_delimiter(char* buffer, DavixError** err){
    char* p;
    if( (p =strchr(buffer, ':')) == NULL){
      HttpIoVecSetupErrorMultiPart(err);
    }
    return p;
}




// analyze header and try to find size of the part
// return 0 -> not a content length header, return -1 : not a header or error, return 1 : success
int find_header_params(char* buffer, dav_size_t* part_size, DavixError** err){
  /*  char * p = header_delimiter(buffer, err);
    if(p == NULL)
        return -1;
    std::string header_type(buffer, p - buffer);
    if(ans_header_content_length.compare(0, p - buffer, buffer) !=0)
        return 0;
    p++;
    while( (*p == ' ' || *p == '\t') && *p != '\0' )
        p++;

    long chunk_size = strtol(p, NULL, 10);
    if(chunk_size == LONG_MAX || chunk_size <= 0)
        return -1;
    *part_size = chunk_size;*/
    return 1;
}



dav_ssize_t find_and_analyze_multi_part_headers(HttpRequest& req, const std::string & boundary,
                                                dav_ssize_t current_offset,
                                                const dav_ssize_t request_size,
                                                dav_size_t* part_size,
                                                dav_off_t* part_offset, DavixError** err){
    dav_ssize_t tmp_read_size =0;
    char buffer[DAVIX_READ_BLOCK_SIZE+1];
    buffer[DAVIX_READ_BLOCK_SIZE]= '\0';

    while((tmp_read_size = req.readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) > 0
          && (current_offset += tmp_read_size) < request_size){


    }
    HttpIoVecSetupErrorMultiPart(err);
    return -1;
}


dav_ssize_t find_next_part(HttpRequest& req, const std::string & boundary,
                                    dav_ssize_t current_offset,
                                    const dav_ssize_t request_size,
                                    dav_size_t* part_size,
                                    dav_off_t* part_offset, DavixError** err){
    dav_ssize_t tmp_read_size =0;
    char buffer[DAVIX_READ_BLOCK_SIZE+1];
    buffer[DAVIX_READ_BLOCK_SIZE]= '\0';

    // read first line, check and try to find boundary pattern
    if( (tmp_read_size = req.readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) <0 )
        return -1;

    current_offset +=tmp_read_size;

    if(tmp_read_size == DAVIX_READ_BLOCK_SIZE || current_offset >= request_size){
        HttpIoVecSetupErrorMultiPart(err);
        return -1;
    }

    if( is_a_start_boundary_part(buffer, tmp_read_size, boundary, err) == false)
        return -1;
    return find_and_analyze_multi_part_headers(req, boundary, current_offset,
                                               request_size, part_size, part_offset, err);

}



ssize_t HttpVecOps::parseMultipartRequest(const DavIOVecInput *input_vec,
                                            DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err){
    std::string boundary;
    dav_ssize_t file_size;
    dav_ssize_t current_offset=0;
    ssize_t ret = 0;

    if(( file_size = _req.getAnswerSize() )== -1
          || check_multi_part_content_type(_req, boundary, err)  != 0 ){
        DAVIX_TRACE("Invalid Header Content info for multi part request");
        HttpIoVecSetupErrorMultiPart(err);
        return -1;
    }

    std::deque<PartPtr> parts;
    for(dav_size_t i =0; i < count_vec; ++i)
        parts.push_back(PartPtr(input_vec+i, output_vec+i));

    while(ret == 0 && parts.size() > 0){
        dav_size_t part_size;
        dav_off_t part_off_set;

        if( ( current_offset = find_next_part(_req, boundary, current_offset, file_size, &part_size, &part_off_set, err)) < 0){

        }
    }

}





/*









dav_ssize_t analyze_multi_part_header(MultiPartPartInfo & mpa, Multipart_analyzer a, DavixError** err){
    char* p = strchr(mpa.buffer,':');
    if(p != NULL){
        if(strncasecmp(ans_header_byte_range, mpa.buffer, ans_header_byte_range.size()) ==0){

            return analyze_skip_header(mpa, analyze_skip_header, err);
        }
        return parse_multi_part_find_next_part(mpa, &analyze_multi_part_header, err);
    }
    DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                           "invalid multi part header field");
    return -1;
}



dav_ssize_t analyze_multi_part_boundary(MultiPartPartInfo & mpa, Multipart_analyzer a, DavixError** err){
    char* p = mpa.buffer;
    while( (p == ' ' || p == '\t'
           || p == '-')
           && p < mpa.buffer + DAVIX_READ_BLOCK_SIZE)
        p++;
    if(strcmp(p, mpa.boundary.c_str()) ==0)
        return parse_multi_part_find_next_part(mpa, analyze_multi_part_header, err);

    DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                           "invalid boundary field");
    return -1;
}


dav_ssize_t analyze_skip_header(MultiPartPartInfo & mpa, Multipart_analyzer a, DavixError** err){
    dav_ssize_t tmp_read_size;

   if( (tmp_read_size = mpa.req->readLine(mpa.buffer, DAVIX_READ_BLOCK_SIZE, err)) <0 )
       return -1;

   mpa.read_number_bytes+= tmp_read_size;
   if(tmp_read_size == DAVIX_READ_BLOCK_SIZE || mpa.read_number_bytes >= mpa.max_size
           || mpa.n_trial++ > mpa.max_trial_number){
       DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                              "Corrupted multi part content");
       return -1;
   }

   if(tmp_read_size == 0) //CRLF -> content start
       return mpa.read_number_bytes;
   return a(mpa, NULL, err);
}



dav_ssize_t skip_crlf(MultiPartPartInfo & mpa, char* buffer, size_t s_buff, Multipart_analyzer* a, DavixError** err){
    if(s_buff != 0 || a == NULL){
        DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                               "No");
        return -1;
    }
}

dav_ssize_t analyse_get_line_content(MultiPartPartInfo & mpa, char* buffer, size_t s_buff, Multipart_analyzer* a, DavixError** err){
    dav_ssize_t tmp_read_size;

    char buffer[DAVIX_READ_BLOCK_SIZE];

    // read a line and trigger next analyzer
   if( (tmp_read_size = mpa.req->readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) <0 )
       return -1;

    mpa.read_number_bytes+= tmp_read_size;
    if(tmp_read_size == DAVIX_READ_BLOCK_SIZE || mpa.read_number_bytes >= mpa.max_size
            || mpa.n_trial++ > mpa.max_trial_number || a == NULL){
        DavixError::setupError(err, davix_scope_io_buff(), StatusCode::InvalidServerResponse,
                               "Corrupted multi part content");
        return -1;
    }

    return *a(mpa, buffer, tmp_read_size, a+1, err);
}








}*/


} // Davix
