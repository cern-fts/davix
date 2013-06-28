#include "fileutils.hpp"
#include <sstream>
namespace Davix {


const std::string ans_header_byte_range("Content-Range");
const std::string ans_header_content_type("Content-Type");
const std::string ans_header_multi_part_value("multipart");
const std::string ans_header_boundary_field("boundary=");
const std::string ans_header_content_length("Content-Length");


int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err){
    const int reqcode = req->getRequestCode();
    int ret = 0;
    if( httpcodeIsValid(reqcode) == false){
        DavixError* tmp_err=NULL;
        httpcodeToDavixCode(reqcode, scope, "",&tmp_err);
        if(tmp_err && tmp_err->getStatus() != StatusCode::OK){
            DavixError::propagateError(err, tmp_err);
            ret = -1;
        }else{
            DavixError::clearError(&tmp_err);
        }
    }
    return ret;
}


void setup_offset_request(HttpRequest* req, const dav_off_t *start_len, const dav_size_t *size_read, const dav_size_t number_ops){
    static const std::string offset_value("bytes=");
    static const std::string req_header_byte_range("Range");
    std::ostringstream buffer;
    buffer << offset_value;

    for(size_t i = 0; i<number_ops; ++i){
        if( i > 0)
            buffer << ",";

       if(size_read > 0)
           buffer << start_len[i] << "-"<< (start_len[i]+size_read[i]-1);
       else
            buffer << start_len [i]<< "-";
     }
     req->addHeaderField(req_header_byte_range, buffer.str());

}

void fill_stat_from_fileproperties(struct stat* st, const  FileProperties & prop){
    memset(st, 0, sizeof(struct stat));
    st->st_mtime = prop.mtime;
    st->st_atime = prop.atime;
    st->st_ctime = prop.ctime;
    st->st_size = prop.size;
    st->st_mode = prop.mode;
}


} // namespace Davix
