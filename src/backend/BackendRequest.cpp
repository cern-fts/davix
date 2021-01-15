/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgios.bitzes@cern.ch>
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

#include "BackendRequest.hpp"
#include <core/ContentProvider.hpp>
#include <utils/davix_s3_utils.hpp>
#include <utils/davix_azure_utils.hpp>
#include <utils/davix_gcloud_utils.hpp>
#include <utils/davix_swift_utils.hpp>
#include <utils/davix_logger_internal.hpp>
#include <utils/stringutils.hpp>
#include <fileops/fileutils.hpp>
#include <string>

namespace Davix {

//------------------------------------------------------------------------------
// Default constructor
//------------------------------------------------------------------------------
BackendRequest::BackendRequest(Context &c, const Uri &uri)
  : _context(c), _current( new Uri(uri)),
    _orig(_current),
    _params(),
    _request_type("GET"),
    _req_flag(RequestFlag::IdempotentRequest),
    _deadline(),
    _content_provider(NULL),
    _ans_size(-1),
    _early_termination(false),
    _early_termination_error(NULL) {}

//------------------------------------------------------------------------------
// Virtual destructor
//------------------------------------------------------------------------------
BackendRequest::~BackendRequest() {}

//------------------------------------------------------------------------------
// Several different ways to provide the request body.
//------------------------------------------------------------------------------
void BackendRequest::setRequestBody(const std::string & body) {
  _owned_content_provider.reset(new OwnedBufferContentProvider(body));
  _content_provider = _owned_content_provider.get();
}

void BackendRequest::setRequestBody(const void * buffer, dav_size_t len) {
  _owned_content_provider.reset(new BufferContentProvider( (const char*) buffer, len));
  _content_provider = _owned_content_provider.get();
}

void BackendRequest::setRequestBody(int fd, dav_off_t offset, dav_size_t len) {
  _owned_content_provider.reset(new FdContentProvider(fd, offset, len));
  _content_provider = _owned_content_provider.get();
}

void BackendRequest::setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata) {
  _owned_content_provider.reset(new CallbackContentProvider(provider, len, udata));
  _content_provider = _owned_content_provider.get();
}

void BackendRequest::setRequestBody(ContentProvider &provider) {
  _content_provider = &provider;
}

//------------------------------------------------------------------------------
// Set request parameters.
//------------------------------------------------------------------------------
void BackendRequest::setParameters(const RequestParams &p) {
  _params = p;
}

//------------------------------------------------------------------------------
// Get request parameters.
//------------------------------------------------------------------------------
RequestParams& BackendRequest::getParameters() {
  return _params;
}

//------------------------------------------------------------------------------
// Configure request for S3.
//------------------------------------------------------------------------------
void BackendRequest::configureS3params() {
  // strange workaround to get S3 compatibility on gcloud to work
  if(_params.getAwsRegion().empty()) {
    HeaderVec vec = _headers_field;
    S3::signRequest(_params, _request_type, *_current, vec);
    vec.swap(_headers_field);
  }
  else {
    Uri signed_url = S3::signURI(_params, _request_type, *_current, _headers_field, DEFAULT_REQUEST_SIGNING_DURATION);
    _current= std::shared_ptr<Uri>(new Uri(signed_url));
  }
}

//------------------------------------------------------------------------------
// Configure request for Azure.
//------------------------------------------------------------------------------
void BackendRequest::configureAzureParams() {
  Uri signed_url = Azure::signURI(_params.getAzureKey(), _request_type, *_current, DEFAULT_REQUEST_SIGNING_DURATION);
  _current.reset(new Uri(signed_url));
}

//------------------------------------------------------------------------------
// Configure request for Gcloud.
//------------------------------------------------------------------------------
void BackendRequest::configureGcloudParams() {
  Uri signed_url = gcloud::signURI(_params.getGcloudCredentials(), _request_type, *_current, _headers_field, DEFAULT_REQUEST_SIGNING_DURATION);
  _current.reset(new Uri(signed_url));
}

//------------------------------------------------------------------------------
// Configure request for Swift.
//------------------------------------------------------------------------------
void BackendRequest::configureSwiftParams() {
    _headers_field.emplace_back("X-Auth-Token", _params.getOSToken());
    Uri signed_url = Swift::signURI(_params, *_current);
    _current.reset(new Uri(signed_url));
}

//------------------------------------------------------------------------------
// Set-up deadline, but only if uninitialized
//------------------------------------------------------------------------------
void BackendRequest::setupDeadlineIfUnset() {
  if(_deadline.isValid() == false && _params.getOperationTimeout()->tv_sec != 0){
    using namespace Chrono;
    _deadline = Clock(Clock::Monolitic).now() + Duration(_params.getOperationTimeout()->tv_sec);
  }
}

//------------------------------------------------------------------------------
// Check if deadline has already passed
//------------------------------------------------------------------------------
bool BackendRequest::checkTimeout(DavixError **err){
  if(_deadline.isValid() && _deadline < Chrono::Clock(Chrono::Clock::Monolitic).now()) {
    std::ostringstream ss;
    ss << "timeout of " << _params.getOperationTimeout()->tv_sec << "s";
    DavixError::setupError(err, davix_scope_http_request(), StatusCode::OperationTimeout, ss.str());
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
// Get answer size from headers.
//------------------------------------------------------------------------------
dav_ssize_t BackendRequest::getAnswerSizeFromHeaders() const {
  std::string str_file_size="";

  long size=-1;
  if(getAnswerHeader(ans_header_content_length, str_file_size)) {
    StrUtil::trim(str_file_size);
    try{
      size = toType<long, std::string>()(str_file_size);
    } catch(...){
      size = -1;
    }
  }

  if(size == -1){
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "Bad server answer: {} Invalid, impossible to determine answer size", ans_header_content_length);
  }

  return static_cast<dav_ssize_t>(size);
}

//------------------------------------------------------------------------------
// Get this requests' context.
//------------------------------------------------------------------------------
Context& BackendRequest::getContext() const {
  return _context;
}

//------------------------------------------------------------------------------
// Get original URL, before any redirections
//------------------------------------------------------------------------------
std::shared_ptr<Uri> BackendRequest::getOriginalUri() const {
  return _orig;
}

//------------------------------------------------------------------------------
// Helper read members - implemented in terms of readBlock, and an internal
// buffer.
//------------------------------------------------------------------------------
dav_ssize_t BackendRequest::readSegment(char* p_buff, dav_size_t size_read, bool stop_at_line_boundary, DavixError**err) {
  DavixError* tmp_err=NULL;
  dav_ssize_t ret, tmp_ret;
  dav_size_t s_read= size_read;
  ret = tmp_ret = 0;
  DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "Davix::BackendRequest::readSegment: want to read {} bytes ", size_read);
  bool early_stop = false;

  do {
    tmp_ret= readBlock(p_buff, s_read, &tmp_err);

    if(tmp_ret > 0 && stop_at_line_boundary && std::find(p_buff, p_buff+tmp_ret, '\n') != p_buff+tmp_ret) {
      early_stop = true;
    }

    if(tmp_ret > 0){ // tmp_ret bytes read
      ret += tmp_ret;
    }

    if(ret > 0 && ret < (dav_ssize_t) size_read){
      p_buff+= tmp_ret;
      s_read -= tmp_ret;
    }

  } while( !early_stop && tmp_ret > 0 && ret < (dav_ssize_t) size_read);

  if(tmp_err){
    DavixError::propagateError(err, tmp_err);
    return -1;
  }

  return ret;
}

dav_ssize_t BackendRequest::readToFd(int fd, dav_size_t read_size, DavixError** err){
  dav_ssize_t ret=1, total=0;
  dav_size_t chunk_size = DAVIX_BLOCK_SIZE;
  read_size = (read_size==0)?(std::numeric_limits<dav_size_t>::max()):read_size;
  std::vector<char> buffer(chunk_size);

  while( (ret = readBlock(&buffer[0],
                          std::min<dav_size_t>(chunk_size,read_size),
                          err)) >0 && ( read_size >0 )){
    if(((dav_size_t)ret) == chunk_size && chunk_size < DAVIX_MAX_BLOCK_SIZE){ // increase buffer size
      chunk_size = std::min<dav_size_t>(chunk_size << 1, DAVIX_MAX_BLOCK_SIZE);
      buffer.resize(chunk_size);
    }

    dav_ssize_t write_len = ret;
    read_size -= ret;
    total += ret;

    do {
      ret = write(fd, &buffer[0], write_len);

      if (ret == -1 && errno == EINTR) {
        continue;
      } else if (ret < 0) {
        DavixError::setupError(err, davix_scope_http_request(),
            StatusCode::SystemError, std::string("Impossible to write to fd").append(strerror(errno)));
        return -1;
      } else {
        write_len -= ret;
      }
    } while(write_len >0);
  }

  if(total > 0) return total;
  return ret;
}

dav_ssize_t BackendRequest::readLine(char* buffer, dav_size_t max_size, DavixError** err){
  dav_ssize_t ret=-1;

  if( _vec_line.size() > 0){
    std::vector<char>::iterator it;
    it = std::find(_vec_line.begin(), _vec_line.end(), '\n');

    if( it  != _vec_line.end()){
      it++;
      const dav_ssize_t read_size = it - _vec_line.begin();
      std::copy(_vec_line.begin(), it, buffer);
      _vec_line.erase(_vec_line.begin(), it);
      return read_size;
    }

    std::copy(_vec_line.begin(), _vec_line.end(), buffer);
    const dav_ssize_t n_bytes =  _vec_line.size();
    _vec_line.clear();
    ret = readLine(buffer + n_bytes, max_size - n_bytes, err);
    return (ret >= 0)?(ret + n_bytes):-1;
  }

  if( ( ret = readSegment(buffer, max_size, true, err)) >= 0){
    // search for crlf
    char* p_endline;
    p_endline = std::find(buffer, buffer+ret, '\n');
    if( p_endline < buffer+ret) p_endline++;
    _vec_line.reserve(ret - ( p_endline - buffer ));
    std::copy(p_endline, buffer + ret, std::back_inserter(_vec_line));
    *p_endline = '\0';
    return  p_endline - buffer;
  }

  return -1;
}

//------------------------------------------------------------------------------
// Access response buffer.
//------------------------------------------------------------------------------
const char* BackendRequest::getAnswerContent() {
  if(_vec.size() != 0) {
    return (const char*) &(_vec.at(0));
  }

  return NULL;
}

std::vector<char>& BackendRequest::getAnswerContentVec() {
  return _vec;
}

//------------------------------------------------------------------------------
// Clear response buffer.
//------------------------------------------------------------------------------
void BackendRequest::clearAnswerContent() {
  _vec.clear();
}

//------------------------------------------------------------------------------
// Parse "Last-Modified" response header, return as time_t.
//------------------------------------------------------------------------------
time_t BackendRequest::getLastModified() const {
  time_t t=0;

  std::string str_lastmodified;
  if(getAnswerHeader("Last-Modified", str_lastmodified)) {
    StrUtil::trim(str_lastmodified);
    try{
      t = S3::s3TimeConverter(str_lastmodified);
    } catch(...){
      str_lastmodified.clear();
    }
  }

  if(str_lastmodified.empty()){
    DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "Bad server answer: {} Invalid, impossible to determine last modified time");
  }

  return t;
}

//------------------------------------------------------------------------------
// Get answer size
//------------------------------------------------------------------------------
dav_ssize_t BackendRequest::getAnswerSize() const {
  if(_ans_size < 0) {
    _ans_size = getAnswerSizeFromHeaders();
  }

  return _ans_size;
}


}