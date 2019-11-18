/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#include "StandaloneCurlRequest.hpp"
#include "CurlSessionFactory.hpp"
#include "CurlSession.hpp"
#include "HeaderlineParser.hpp"
#include <utils/davix_logger_internal.hpp>
#include <core/ContentProvider.hpp>
#include <curl/curl.h>
#include <auth/davixx509cred_internal.hpp>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()
#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

namespace Davix {

static std::vector<std::string> split(std::string data, std::string token) {
  std::vector<std::string> output;
  size_t pos = std::string::npos;
  do {
    pos = data.find(token);
    output.push_back(data.substr(0, pos));
    if(std::string::npos != pos) data = data.substr(pos + token.size());
  } while (std::string::npos != pos);
  return output;
}

static std::string chopNewline(const std::string &line) {
  if(line.size() > 2 && line[line.size()-2] == '\r' && line[line.size()-1] == '\n') {
    return std::string(line.begin(), line.begin()+line.size()-2);
  }

  if(line.size() > 1 && line[line.size()-1] == '\n') {
    return std::string(line.begin(), line.begin()+line.size()-1);
  }

  return line;
}

//------------------------------------------------------------------------------
// Debug callback
//------------------------------------------------------------------------------
int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
  static bool prevHeaderOut = false;
  static bool prevHeaderIn = false;

  switch(type) {
    case CURLINFO_HEADER_IN: {
      prevHeaderOut = false;

      if(!prevHeaderIn) {
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HEADER, "");
        prevHeaderIn = true;
      }

      if(::Davix::getLogScope() & DAVIX_LOG_HEADER) {
        std::vector<std::string> lines = split(std::string(data, size), "\r\n");
        for(size_t i = 0; i < lines.size(); i++) {
          std::string chopped = chopNewline(lines[i]);

          if(!chopped.empty()) {
            DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HEADER, "> {}", chopped);
          }

        }
      }

      break;
    }
    case CURLINFO_HEADER_OUT: {
      prevHeaderIn = false;

      if(!prevHeaderOut) {
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HEADER, "");
        prevHeaderOut = true;
      }

      if(::Davix::getLogScope() & DAVIX_LOG_HEADER) {
        std::vector<std::string> lines = split(std::string(data, size), "\r\n");
        for(size_t i = 0; i < lines.size(); i++) {
          std::string chopped = chopNewline(lines[i]);

          if(!chopped.empty()) {
            DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HEADER, "< {}", chopped);
          }
        }
      }

      break;
    }
    case CURLINFO_DATA_IN: {
      DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_BODY, "Body block incoming ({} bytes): {}", size, std::string(data, size));
      break;
    }
    case CURLINFO_DATA_OUT: {
      DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_BODY, "Body block outgoing ({} bytes): {}", size, std::string(data, size));
      break;
    }

    default: {}
  }

  return 0;
}

//------------------------------------------------------------------------------
// Header callback
//------------------------------------------------------------------------------
static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
  size_t bytes = size * nitems;

  StandaloneCurlRequest* req = (StandaloneCurlRequest*) userdata;
  req->feedResponseHeader(std::string(buffer, bytes));
  return bytes;
}

//------------------------------------------------------------------------------
// Write callback
//------------------------------------------------------------------------------
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t bytes = size * nmemb;

  ResponseBuffer* buff = (ResponseBuffer*) userdata;
  buff->feed(ptr, bytes);
  return bytes;
}

//------------------------------------------------------------------------------
// Read callback
//------------------------------------------------------------------------------
size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
  size_t bytes = size * nitems;

  ContentProvider* provider = (ContentProvider*) userdata;
  ssize_t retval = provider->pullBytes(buffer, bytes);

  if(retval < 0) {
    DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HTTP, "Content provider reported an errc={}", retval);
    return 0;
  }

  return retval;
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
StandaloneCurlRequest::StandaloneCurlRequest(CurlSessionFactory &sessionFactory, bool reuseSession,
  const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
  const std::vector<HeaderLine> &headers, int reqFlag, ContentProvider *contentProvider,
  Chrono::TimePoint deadline)
: _session_factory(sessionFactory), _reuse_session(reuseSession), _bound_hooks(boundHooks),
  _uri(uri), _verb(verb), _params(params), _headers(headers), _req_flag(reqFlag),
  _content_provider(contentProvider), _deadline(deadline), _state(RequestState::kNotStarted),
  _chunklist(NULL), _received_headers(false) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
StandaloneCurlRequest::~StandaloneCurlRequest() {
    curl_slist_free_all(_chunklist);
}

//------------------------------------------------------------------------------
// Get a specific response header
//------------------------------------------------------------------------------
bool StandaloneCurlRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {
  for(auto it = _response_headers.begin(); it != _response_headers.end(); it++) {
    if(it->first == header_name) {
      value = it->second;
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------------------
// Get all response headers
//------------------------------------------------------------------------------
size_t StandaloneCurlRequest::getAnswerHeaders(std::vector<std::pair<std::string, std::string > > & vec_headers) const {
  vec_headers = _response_headers;
  return vec_headers.size();
}

//------------------------------------------------------------------------------
// Traslate curl code into status
//------------------------------------------------------------------------------
static Status curlCodeToStatus(CURLcode code) {
  switch(code) {
    case CURLE_OK: {
      return Status();
    }
    default: {
      std::ostringstream ss;
      ss << "curl error (" << code << "): " << curl_easy_strerror(code);
      return Status(davix_scope_http_request(), StatusCode::UnknowError, ss.str());
    }
  }
}

//------------------------------------------------------------------------------
// Get curl version as string
//------------------------------------------------------------------------------
static std::string getCurlVersion() {
  curl_version_info_data* ver = curl_version_info(CURLVERSION_NOW);
  return ver->version;
}

//------------------------------------------------------------------------------
// Start request - calling this multiple times will do nothing.
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::startRequest() {
  if(_state != RequestState::kNotStarted) {
    return Status(); ;
  }

  //----------------------------------------------------------------------------
  // Have we timed out already?
  //----------------------------------------------------------------------------
  Status st = checkTimeout();
  if(!st.ok()) {
    // markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Retrieve a session, create request
  //----------------------------------------------------------------------------
  _session = _session_factory.provideCurlSession(_uri, _params, st);
  if(!st.ok()) {
    // markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Set request verb, target URL
  //----------------------------------------------------------------------------
  CURL* handle = _session->getHandle()->handle;
  CURLM* mhandle = _session->getHandle()->mhandle;

  Uri uriCopy(_uri);
  uriCopy.httpizeProtocol();

  curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, _verb.c_str());
  curl_easy_setopt(handle, CURLOPT_URL, uriCopy.getString().c_str());

  //----------------------------------------------------------------------------
  // Set up callback to consume response headers
  //----------------------------------------------------------------------------
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

  //----------------------------------------------------------------------------
  // Set up callback to consume response body
  //----------------------------------------------------------------------------
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &_response_buffer);

  //----------------------------------------------------------------------------
  // Set up callback to provide request body
  //----------------------------------------------------------------------------
  if(_content_provider) {
    _content_provider->rewind();
    curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(handle, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(handle, CURLOPT_READDATA, _content_provider);
    curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, _content_provider->getSize());

  }

  //----------------------------------------------------------------------------
  // Set up CA store
  //----------------------------------------------------------------------------
  if(_params.getSSLCACheck()) {
    for(auto it = _params.listCertificateAuthorityPath().begin(); it != _params.listCertificateAuthorityPath().end(); it++) {
      struct stat st;
      if (stat(it->c_str(), &st) < 0 || S_ISDIR(st.st_mode) == false) {
        DAVIX_SLOG(DAVIX_LOG_WARNING, DAVIX_LOG_HTTP, "CA Path invalid : {}, {} ", *it, ((errno != 0)?strerror(errno):strerror(ENOTDIR)));
        errno = 0;
      } else {
        DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "add CA PATH {}", *it);
        curl_easy_setopt(handle, CURLOPT_CAPATH, it->c_str());
      }
    }
  }
  else {
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
  }

  //----------------------------------------------------------------------------
  // Set up client certificate
  //----------------------------------------------------------------------------
  const X509Credential& clientCert = _params.getClientCertX509();

  std::string userCertificate;
  std::string userKey;
  std::string password;

  if(X509CredentialExtra::get_x509_info(clientCert, &userCertificate, &userKey, &password)) {
    curl_easy_setopt(handle, CURLOPT_SSLCERT, userCertificate.c_str());
    curl_easy_setopt(handle, CURLOPT_SSLKEY,  userKey.c_str());
  }

  //----------------------------------------------------------------------------
  // Set up headers
  //----------------------------------------------------------------------------
  for(size_t i = 0; i < _headers.size(); i++) {
    _chunklist = curl_slist_append(_chunklist, SSTR(_headers[i].first << ": " << _headers[i].second).c_str());

    if(_content_provider && _params.get100ContinueSupport() && (_req_flag & RequestFlag::SupportContinue100) ) {
      _chunklist = curl_slist_append(_chunklist, "Expect: 100-continue");
    }
    else {
      _chunklist = curl_slist_append(_chunklist, "Expect:");
    }
  }

  _chunklist = curl_slist_append(_chunklist, SSTR("User-Agent: " << Davix::RequestParams().getUserAgent() << " libcurl/" << getCurlVersion()).c_str());

  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, _chunklist);

  //----------------------------------------------------------------------------
  // Set up debugging
  //----------------------------------------------------------------------------
  curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, debug_callback);
  curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);

  //----------------------------------------------------------------------------
  // Start request
  //----------------------------------------------------------------------------
  int still_running = 1;

  while(still_running != 0) {
    if(curl_multi_perform(mhandle, &still_running) != CURLM_OK) {
      break;
    }
  }

  _state = RequestState::kStarted;

  CURLMsg *msg;
  int msgs_left = 0;
  while(msg = curl_multi_info_read(mhandle, &msgs_left)) {
    if(msg->msg == CURLMSG_DONE && msg->data.result != CURLE_OK) {
      return curlCodeToStatus(msg->data.result);
    }
  }

  return Status();
}

//------------------------------------------------------------------------------
// Major read function - read a block of max_size bytes (at max) into buffer.
//------------------------------------------------------------------------------
dav_ssize_t StandaloneCurlRequest::readBlock(char* buffer, dav_size_t max_size, Status& st) {
  if(!_session) {
    st = Status(davix_scope_http_request(), StatusCode::AlreadyRunning, "Request has not been started yet");
    return -1;
  }

  if(max_size == 0) {
    return 0;
  }

  st = checkTimeout();
  if(!st.ok()) {
    return -1;
  }

  st = Status();
  return _response_buffer.consume(buffer, max_size);
}

//------------------------------------------------------------------------------
// Finish an already started request.
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::endRequest() {
  _state = RequestState::kFinished;
  return Status();
}

//------------------------------------------------------------------------------
// Check request state
//------------------------------------------------------------------------------
RequestState StandaloneCurlRequest::getState() const {
  return _state;
}

//------------------------------------------------------------------------------
// Check if timeout has passed
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::checkTimeout() {
  if(_deadline.isValid() && _deadline < Chrono::Clock(Chrono::Clock::Monolitic).now()) {
    std::ostringstream ss;
    ss << "timeout of " << _params.getOperationTimeout()->tv_sec << "s";
    return Status(davix_scope_http_request(), StatusCode::OperationTimeout, ss.str());
  }

  return Status();
}

//------------------------------------------------------------------------------
// Get status code - returns 0 if impossible to determine
//------------------------------------------------------------------------------
int StandaloneCurlRequest::getStatusCode() const {
  long response_code = 0;

  if(_session) {
    CURL* handle = _session->getHandle()->handle;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
  }

  return response_code;
}

//------------------------------------------------------------------------------
// Do not re-use underlying session
//------------------------------------------------------------------------------
void StandaloneCurlRequest::doNotReuseSession() {

}

//------------------------------------------------------------------------------
// Has the underlying session been used before?
//------------------------------------------------------------------------------
bool StandaloneCurlRequest::isRecycledSession() const {

}

//------------------------------------------------------------------------------
// Obtain redirected location, store into the given Uri
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::obtainRedirectedLocation(Uri &out) {
  if(!_session) {
    return Status(davix_scope_http_request(), StatusCode::InvalidArgument, "Request not active, impossible to obtain redirected location");
  }

  for(auto it = _response_headers.begin(); it != _response_headers.end(); it++) {
    if(strcasecmp("location", it->first.c_str()) == 0) {
      out = Uri(it->second);
      return Status();
    }
  }

  return Status(davix_scope_http_request(), StatusCode::InvalidArgument, "Could not find Location header in answer headers");
}

//------------------------------------------------------------------------------
// Get session error, if available
//------------------------------------------------------------------------------
std::string StandaloneCurlRequest::getSessionError() const {

}

//------------------------------------------------------------------------------
// Block until all response headers have been received
//------------------------------------------------------------------------------
Status StandaloneCurlRequest::readResponseHeaders() {

}

//------------------------------------------------------------------------------
// Feed response header
//------------------------------------------------------------------------------
void StandaloneCurlRequest::feedResponseHeader(const std::string &header) {
  if(header == "\r\n") {
    _received_headers = true;
    return;
  }

  HeaderlineParser parser(header);
  _response_headers.push_back(std::pair<std::string, std::string>(parser.getKey(), parser.getValue()));
}


}