/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2013
 * Author: Adrien Devresse <adrien.devresse@cern.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include <davix_internal.hpp>
#include "fileutils.hpp"

namespace Davix {


const std::string ans_header_byte_range("Content-Range");
const std::string ans_header_content_type("Content-Type");
const std::string ans_header_multi_part_value("multipart");
const std::string ans_header_boundary_field("boundary=");
const std::string ans_header_content_length("Content-Length");
const std::string offset_value("bytes=");
const std::string req_header_byte_range("Range");


int davixRequestToFileStatus(HttpRequest* req, const std::string & scope, DavixError** err){
    const int reqcode = req->getRequestCode();
    int ret = 0;
    if( httpcodeIsValid(reqcode) == false){
        DavixError* tmp_err=NULL;
        httpcodeToDavixError(reqcode, scope, "",&tmp_err);
        if(tmp_err && tmp_err->getStatus() != StatusCode::OK){
            DavixError::propagateError(err, tmp_err);
            ret = -1;
        }else{
            DavixError::clearError(&tmp_err);
        }
    }
    return ret;
}


void check_file_status(HttpRequest & req, const std::string & scope){
    const int code =req.getRequestCode();
    if( httpcodeIsValid(code) == false){
        httpcodeToDavixException(code, scope);
    }
}

void setup_offset_request(HttpRequest* req, const dav_off_t *start_len, const dav_size_t *size_read, const dav_size_t number_ops){
   std::ostringstream buffer;

    buffer << offset_value;

    for(size_t i = 0; i<number_ops; ++i){
        if( i > 0)
            buffer << ",";

       if(size_read[i] > 0)
           buffer << start_len[i] << "-"<< (start_len[i]+size_read[i]-1);
       else
            buffer << start_len [i]<< "-";
     }
     req->addHeaderField(req_header_byte_range, buffer.str());

}

std::vector< std::pair<dav_size_t, std::string> > generateRangeHeaders(dav_size_t max_header_size, OffsetCallback & offset_provider){
   std::vector< std::pair<dav_size_t, std::string> > range_rec;
   dav_off_t begin, end;
   int ret;
   std::string range_string;
   dav_size_t range_size =0;
   std::ostringstream buffer;
   range_string.reserve(max_header_size);
   range_string.append(offset_value);


   while( ( ret = offset_provider(begin, end)) >= 0){
      buffer.str("");

      /* First range? Don't add a coma */
      if(range_string.size() != offset_value.size())
          buffer << ",";
      buffer << begin << '-' <<  end;

      range_string.append(buffer.str());
      range_size++;
      if(range_string.size() >= max_header_size){
          range_rec.push_back(std::make_pair(range_size, range_string));
          range_size = 0;
          range_string.assign(offset_value);
      }
   }
   if(range_size > 0)
       range_rec.push_back(std::make_pair(range_size, range_string));

   return range_rec;
}





} // namespace Davix
