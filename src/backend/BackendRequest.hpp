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

#ifndef BACKEND_REQUEST_HPP
#define BACKEND_REQUEST_HPP

#include <davix_internal.hpp>
#include <request/httprequest.hpp>
#include <utils/davix_uri.hpp>
#include <memory>

#define DEFAULT_REQUEST_SIGNING_DURATION 3600

namespace Davix{

//------------------------------------------------------------------------------
// Describe current request status.
//------------------------------------------------------------------------------
enum class RequestStatus {
  kNotStarted = 0,      // request is not active
  kStarted,             // request has been started
  kCompletedOneShot,    // request completed through 'executeRequest', not begin/end
};

//------------------------------------------------------------------------------
// Abstract HTTP request type towards a backend.
//------------------------------------------------------------------------------
class BackendRequest {
public:
  //----------------------------------------------------------------------------
  // Default constructor
  //----------------------------------------------------------------------------
  BackendRequest(Context &c, const Uri &uri);

  //----------------------------------------------------------------------------
  // Virtual destructor
  //----------------------------------------------------------------------------
  virtual ~BackendRequest();

  //----------------------------------------------------------------------------
  // No evil constructors, no move semantics.
  //----------------------------------------------------------------------------
  BackendRequest(const BackendRequest& other) = delete;
  BackendRequest(BackendRequest&& other) = delete;
  BackendRequest& operator=(BackendRequest&& other) = delete;
  BackendRequest& operator=(const BackendRequest& other) = delete;

  //----------------------------------------------------------------------------
  // Major read member - implementations need to override.
  // Read a block of max_size bytes (at max) into buffer.
  //----------------------------------------------------------------------------
  virtual dav_ssize_t readBlock(char* buffer, dav_size_t max_size, DavixError** err) = 0;

  //----------------------------------------------------------------------------
  // Get a specific response header
  //----------------------------------------------------------------------------
  virtual bool getAnswerHeader(const std::string &header_name, std::string &value) const = 0;

  //----------------------------------------------------------------------------
  // Get all response headers
  //----------------------------------------------------------------------------
  virtual size_t getAnswerHeaders(std::vector<std::pair<std::string, std::string > > & vec_headers) const = 0;

  //----------------------------------------------------------------------------
  // Start request.
  //----------------------------------------------------------------------------
  virtual int beginRequest(DavixError** err) = 0;

  //----------------------------------------------------------------------------
  // Finish an already started request.
  //----------------------------------------------------------------------------
  virtual int endRequest(DavixError** err) = 0;

  //----------------------------------------------------------------------------
  // Execute request synchronously, and store result in internal buffer.
  //----------------------------------------------------------------------------
  virtual int executeRequest(DavixError** err) = 0;

  //----------------------------------------------------------------------------
  // Get response status.
  //----------------------------------------------------------------------------
  virtual int getRequestCode() = 0;

  //----------------------------------------------------------------------------
  // Helper read members - implemented in terms of readBlock, and an internal
  // buffer.
  //----------------------------------------------------------------------------
  dav_ssize_t readSegment(char* buffer, dav_size_t size_read, bool stop_at_line_boundary, DavixError** err);
  dav_ssize_t readLine(char* buffer, dav_size_t max_size, DavixError** err);
  dav_ssize_t readToFd(int fd, dav_size_t read_size, DavixError** err);

  //----------------------------------------------------------------------------
  // Add custom header to the request, replace an existing one if already
  // exists. If value is empty, the entire header line is removed.
  //----------------------------------------------------------------------------
  void addHeaderField(const std::string & field, const std::string & value){
    _headers_field.emplace_back(field, value);
  }

  //----------------------------------------------------------------------------
  // Set the request verb. (such as GET, POST, PUT, PROPFIND, etc)
  //----------------------------------------------------------------------------
  void setRequestMethod(const std::string &val){
    _request_type = val;
  }

  //----------------------------------------------------------------------------
  // Get the request verb. (such as GET, POST, PUT, PROPFIND, etc)
  //----------------------------------------------------------------------------
  std::string getRequestMethod() {
    return _request_type;
  }

  //----------------------------------------------------------------------------
  // Set the value of a request flag
  //----------------------------------------------------------------------------
  void setFlag(const RequestFlag::RequestFlag flag, bool value) {
    if(value) {
      _req_flag |=  flag;
    }
    else {
      _req_flag &= ~(flag);
    }
  }

  //----------------------------------------------------------------------------
  // Get the value of a request flag
  //----------------------------------------------------------------------------
  bool getFlag(const RequestFlag::RequestFlag flag) const {
    return _req_flag & ((int) flag);
  }

  //----------------------------------------------------------------------------
  // Several different ways to provide the request body.
  //----------------------------------------------------------------------------
  void setRequestBody(const std::string & body);
  void setRequestBody(const void * buffer, dav_size_t len);
  void setRequestBody(int fd, dav_off_t offset, dav_size_t len);
  void setRequestBody(HttpBodyProvider provider, dav_size_t len, void* udata);
  void setRequestBody(ContentProvider &provider);

  //----------------------------------------------------------------------------
  // Set request parameters.
  //----------------------------------------------------------------------------
  void setParameters(const RequestParams &p);

  //----------------------------------------------------------------------------
  // Get request parameters.
  //----------------------------------------------------------------------------
  RequestParams& getParameters();

  //----------------------------------------------------------------------------
  // Get this requests' context.
  //----------------------------------------------------------------------------
  Context& getContext() const;

  //----------------------------------------------------------------------------
  // Get original URL, before any redirections
  //----------------------------------------------------------------------------
  std::shared_ptr<Uri> getOriginalUri() const;

  //----------------------------------------------------------------------------
  // Access response buffer.
  //----------------------------------------------------------------------------
  const char* getAnswerContent();
  std::vector<char> & getAnswerContentVec();

  //----------------------------------------------------------------------------
  // Clear response buffer.
  //----------------------------------------------------------------------------
  void clearAnswerContent();

  //----------------------------------------------------------------------------
  // Parse "Last-Modified" response header, return as time_t.
  //----------------------------------------------------------------------------
  time_t getLastModified() const;

  //----------------------------------------------------------------------------
  // Get answer size
  //----------------------------------------------------------------------------
  dav_ssize_t getAnswerSize() const;

protected:
  //----------------------------------------------------------------------------
  // Configure request for S3.
  //----------------------------------------------------------------------------
  void configureS3params();

  //----------------------------------------------------------------------------
  // Configure request for Azure.
  //----------------------------------------------------------------------------
  void configureAzureParams();

  //----------------------------------------------------------------------------
  // Configure request for Gcloud.
  //----------------------------------------------------------------------------
  void configureGcloudParams();

  //----------------------------------------------------------------------------
  // Configure request for Swift.
  //----------------------------------------------------------------------------
  void configureSwiftParams();

  //----------------------------------------------------------------------------
  // Set-up deadline, but only if uninitialized
  //----------------------------------------------------------------------------
  void setupDeadlineIfUnset();

  //----------------------------------------------------------------------------
  // Check if deadline has already passed
  //----------------------------------------------------------------------------
  bool checkTimeout(DavixError **err);

  //----------------------------------------------------------------------------
  // Get answer size from headers.
  //----------------------------------------------------------------------------
  dav_ssize_t getAnswerSizeFromHeaders() const;

  //----------------------------------------------------------------------------
  // Member variables common to all implementations.
  //----------------------------------------------------------------------------
  Context& _context;
  std::shared_ptr<Uri>  _current, _orig;
  RequestParams _params;
  std::vector<std::pair<std::string, std::string>> _headers_field;
  std::string _request_type;
  int _req_flag;
  Chrono::TimePoint _deadline;

  //----------------------------------------------------------------------------
  // Request content.
  //----------------------------------------------------------------------------
  std::unique_ptr<ContentProvider> _owned_content_provider;
  ContentProvider *_content_provider;

  //----------------------------------------------------------------------------
  // Answer length.
  //----------------------------------------------------------------------------
  mutable dav_ssize_t _ans_size;

  //----------------------------------------------------------------------------
  // Answer buffers.
  //----------------------------------------------------------------------------
  std::vector<char> _vec;
  std::vector<char> _vec_line;

  //----------------------------------------------------------------------------
  // Early termination flag and status.
  //----------------------------------------------------------------------------
  bool _early_termination;
  DavixError* _early_termination_error;

};


}


#endif
