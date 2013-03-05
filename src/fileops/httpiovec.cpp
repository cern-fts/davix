#include <config.h>
#include "httpiovec.hpp"
#include <cstring>
#include <logger/davix_logger_internal.h>
#include <string_utils/stringutils.hpp>
#include <deque>
#include <functional>
#include <map>


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
       DAVIX_TRACE(" -> Davix Vector operation");
       configure_iovec_range_header(_req, input_vec, count_vec);
       if( _req.beginRequest(&tmp_err)  == 0){
           const int retcode = _req.getRequestCode();
           switch(retcode){
                case 206: // multipart req
                    ret = parseMultipartRequest( input_vec,
                                             output_vec, count_vec, &tmp_err);
                    break;
                case 200: // classical req, simulate vector ops
                    // TODO
                    ret = simulateMultiPartRequest(input_vec, output_vec, count_vec, &tmp_err);
                    break;
                default:
                    httpcodeToDavixCode(_req.getRequestCode(),davix_scope_http_request(),", ", &tmp_err);
                    ret = -1;
           }
       }
       if(tmp_err)
           DavixError::propagateError(err, tmp_err);
       DAVIX_TRACE(" <- Davix Vector operation");
       return ret;
}

void HttpIoVecSetupErrorMultiPart(DavixError** err){
    DavixError::setupError(err, HttpIoVec_scope(), StatusCode::InvalidServerResponse, "Invalid Multi-Part HTTP response");
}

inline bool isValidBoundaryChar(const char* p){
    return ( *p != '\0' && *p != '"' && *p != '\t' && *p != '\n');
}


int http_extract_boundary_from_content_type(const std::string & buffer, std::string & boundary, DavixError** err){
    size_t pos_bound;
    static const std::string delimiter = "\";";
    if( (pos_bound= buffer.find(ans_header_boundary_field)) != std::string::npos){
        std::vector<std::string> tokens = stringTokSplit(buffer.substr(pos_bound + ans_header_boundary_field.size()), delimiter);
        if( tokens.size() >= 1
            && tokens[0].size() > 0
            && tokens[0].size() <= 70){
            DAVIX_TRACE("Multi part content type : %s", boundary.c_str());
            std::swap(boundary,tokens[0]);
            return 0;
        }
    }
    HttpIoVecSetupErrorMultiPart(err);
    return -1;
}


int check_multi_part_content_type(const HttpRequest& req, std::string & boundary, DavixError** err){

    std::string buffer;

    if( req.getAnswerHeader(ans_header_content_type, buffer) == true // has content type
           && http_extract_boundary_from_content_type(buffer, boundary, err) == 0){
          return 0;
    }
    HttpIoVecSetupErrorMultiPart(err);
    return -1;
}


bool is_a_start_boundary_part(char* buffer, size_t s_buff, const std::string & boundary,
                            DavixError** err){
    if(s_buff > 3){
        char * p = buffer;
        if( *p == '-' && *(p+1)== '-'){
            if( strcmp(buffer+2, boundary.c_str()) ==0){
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
int find_header_params(char* buffer, dav_size_t* part_size, dav_off_t* part_offset){
    static const std::string delimiter(" bytes-/\t");
    char * p = header_delimiter(buffer, NULL);
    if(p == NULL)
        return -1;
    std::string header_type(buffer, p - buffer);
    if( string_compare_ncase(ans_header_byte_range, 0, p - buffer, buffer) !=0) // check header type
        return 0;

    std::vector<std::string> tokens = stringTokSplit(std::string(p+1),delimiter);     // parse header
    if(tokens.size() != 3)
        return -1;

    long chunk_size[2];
    for(int i =0; i <2;++i){
        chunk_size[i]= strtol(tokens[i].c_str(), &p, 10);
        if(chunk_size[i] == LONG_MAX || chunk_size[i] < 0 || *p != '\0'){
            errno =0;
            return -1;
        }
    }
    if(chunk_size[1] < chunk_size[0])
        return -1;

    *part_offset= chunk_size[0];
    *part_size =  chunk_size[1]-chunk_size[0]+1;
    return 1;
}

dav_ssize_t find_end_header_multi_part(HttpRequest& req, const std::string & boundary,
                                       dav_ssize_t current_offset,
                                       const dav_ssize_t request_size,
                                       DavixError** err){
    char buffer[DAVIX_READ_BLOCK_SIZE];
    dav_ssize_t tmp_read_size =0;
    while((tmp_read_size = req.readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) > 0
          && (current_offset += tmp_read_size) < request_size);
    if(tmp_read_size == 0) // CRLF FOUND !
        return current_offset;
    return -1;
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
            int res = find_header_params(buffer, part_size,part_offset);
            switch(res){
                case 0:
                    break;
                case 1:
                    return find_end_header_multi_part(req, boundary,
                                                          current_offset,
                                                          request_size,
                                                           err);
                    break;
                default:
                    HttpIoVecSetupErrorMultiPart (err);
                    return -1;
            }

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
    int n_try =0;
    char buffer[DAVIX_READ_BLOCK_SIZE+1];
    buffer[DAVIX_READ_BLOCK_SIZE]= '\0';

    // read first line, check and try to find boundary pattern, ignore some empty line if present
    while(n_try++ < 10 && tmp_read_size ==0){
        if( (tmp_read_size = req.readLine(buffer, DAVIX_READ_BLOCK_SIZE, err)) <0 )
            return -1;
    }
    if(n_try >= 10){
        HttpIoVecSetupErrorMultiPart(err);
        return -1;
    }


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


static bool find_corresponding_part(std::deque<PartPtr> & parts, dav_size_t part_size,
                                       dav_off_t part_off_set, std::deque<PartPtr>::iterator & res, DavixError** err){
    for(std::deque<PartPtr>::iterator it = parts.begin(); it != parts.end();it++){
        if( (*it).first->diov_offset == part_off_set &&  (*it).first->diov_size == part_size){
           res= it;
           DAVIX_TRACE(" Match a vec io chunk offset: %ld size: %ld", part_off_set, part_size);
           return true;
        }

    }
    HttpIoVecSetupErrorMultiPart(err);
    return false;
}


ssize_t copy_and_configure(HttpRequest & req, std::deque<PartPtr>::iterator & it, DavixError** err){
    const DavIOVecInput* i = (*it).first;
    DavIOVecOuput* o = (*it).second;
    DavixError* tmp_err=NULL;
    ssize_t ret;
    if( ( ret = read_segment_request(&req, i->diov_buffer, i->diov_size,  0, &tmp_err)) >0){
        o->diov_buffer = i->diov_buffer;
        o->diov_size = ret;
    }
    if(tmp_err){
        // TODO : change error scope
        DavixError::propagateError(err, tmp_err);
    }
    return ret;
}

ssize_t HttpVecOps::parseMultipartRequest(const DavIOVecInput *input_vec,
                                            DavIOVecOuput * output_vec,
                              const dav_size_t count_vec, DavixError** err){
    std::string boundary;
    dav_ssize_t file_size;
    dav_ssize_t current_offset=0;
    ssize_t ret = 0;
    DAVIX_TRACE(" -> Davix multi part parsing");

    if(( file_size = _req.getAnswerSize() )== -1
          || check_multi_part_content_type(_req, boundary, err)  != 0 ){
        DAVIX_TRACE("Invalid Header Content info for multi part request");
        HttpIoVecSetupErrorMultiPart(err);
        return -1;
    }

    std::deque<PartPtr> parts;
    for(dav_size_t i =0; i < count_vec; ++i)
        parts.push_back(PartPtr(input_vec+i, output_vec+i));

    while(ret >= 0 && parts.size() > 0){
        dav_size_t part_size;
        dav_off_t part_off_set;
        ssize_t tmp_ret;

        if( ( current_offset = find_next_part(_req, boundary, current_offset, file_size, &part_size, &part_off_set, err)) < 0){
            return -1;
        }

        std::deque<PartPtr>::iterator part;
        if( find_corresponding_part(parts, part_size, part_off_set, part, err) == false){
            return -1;
        }

        if( (tmp_ret = copy_and_configure(_req, part, err)) < 0)
            return -1;
        ret += tmp_ret;
        parts.erase(part);
    }

    DAVIX_TRACE(" <- Davix multi part parsing");
    return ret;
}



struct ElemChunk{
    ElemChunk(const DavIOVecInput* in, DavIOVecOuput* ou) :
        _in(in),
        _ou(ou),
        _cursor((char*) in->diov_buffer){
        _ou->diov_size=0; // reset elem read status
        _ou->diov_buffer = _in->diov_buffer;
    }

    const DavIOVecInput *_in;
    DavIOVecOuput * _ou;
    char *_cursor;

};

typedef std::pair<dav_off_t, ElemChunk> PairChunk;


typedef std::multimap<dav_off_t, ElemChunk> MapChunk;


// order the chunk by offset
static void fill_map_chunk(MapChunk & m, const DavIOVecInput *input_vec,
                                    DavIOVecOuput * output_vec,
                                    const dav_size_t count_vec){

    for(dav_size_t s = 0; s < count_vec; s++){
        m.insert(PairChunk(input_vec[s].diov_offset, ElemChunk(&input_vec[s], &output_vec[s])));
    }
}


static void balance_iterator_windows(MapChunk & m,
                                     MapChunk::iterator & start, MapChunk::iterator & end,
                                     dav_ssize_t pos, dav_ssize_t read_size){
    dav_ssize_t size_part;
    dav_off_t off_part;
    for(;start != m.end();){ // move the it to first concerned block
        size_part = (*start).second._in->diov_size;
        off_part = (*start).second._in->diov_offset;
        if(pos > ((dav_ssize_t)off_part) + size_part)
            start++;
        else
            break;
    }

    const dav_ssize_t end_chunk_pos = pos + read_size;
    for(;end != m.end();){
        off_part = (*end).second._in->diov_offset;
        if(end_chunk_pos > (dav_ssize_t)off_part )
            end++;
        else
            break;
    }

}


static void fill_concerned_chunk_buffer(MapChunk & m,
                                        MapChunk::iterator & start, MapChunk::iterator & end,
                                        char* buffer, dav_ssize_t read_size, dav_ssize_t pos){

    for(MapChunk::iterator it = start; it != end; it++){
        const dav_ssize_t size_part = (*it).second._in->diov_size;
        const dav_off_t off_part = (*it).second._in->diov_offset;
        const dav_ssize_t cur_chunk_size = (*it).second._ou->diov_size;
        const char* p_buff = (char*) (*it).second._ou->diov_buffer;

        const dav_ssize_t current_chunk_offset = ((dav_ssize_t) off_part + cur_chunk_size);
        const dav_ssize_t read_offset =  current_chunk_offset - pos;
        const dav_ssize_t s_needed = std::min(size_part - cur_chunk_size, read_size - read_offset);
        if(s_needed > 0){
            memcpy((void*) (p_buff + cur_chunk_size), buffer+ read_offset, s_needed);
            (*it).second._ou->diov_size += s_needed;
        }
    }
}

static dav_ssize_t sum_all_chunk_size(const MapChunk & cmap){

    dav_ssize_t res =0;
    for(MapChunk::const_iterator it = cmap.begin(); it != cmap.end(); ++it){
        res += (*it).second._ou->diov_size;
    }
    return res;
}

ssize_t HttpVecOps::simulateMultiPartRequest(const DavIOVecInput *input_vec,
                                 DavIOVecOuput * output_vec,
                   const dav_size_t count_vec, DavixError** err){
    DAVIX_TRACE(" -> Davix vec : 200 full file, simulate vec io");
    MapChunk cmap;
    dav_ssize_t total_read_size=0, tmp_read_size;
    char buffer[DAVIX_READ_BLOCK_SIZE];

    fill_map_chunk(cmap, input_vec, output_vec, count_vec);
    MapChunk::iterator it_start=cmap.begin(),it_end = cmap.begin();
    while( (tmp_read_size = _req.readBlock(buffer, DAVIX_READ_BLOCK_SIZE, err)) >0){
        balance_iterator_windows(cmap, it_start, it_end, total_read_size, tmp_read_size); // re-balance the itnerested windows
        fill_concerned_chunk_buffer(cmap, it_start, it_end, buffer, tmp_read_size, total_read_size); // fill the interested window
        total_read_size += tmp_read_size;
    }
    if(tmp_read_size < 0)
        return -1;

    DAVIX_TRACE(" <- Davix vec : 200 full file, simulate vec io");
    return sum_all_chunk_size(cmap);
}



} // Davix
