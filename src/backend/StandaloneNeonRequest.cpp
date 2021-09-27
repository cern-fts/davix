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

#include "StandaloneNeonRequest.hpp"
#include <status/davixstatusrequest.hpp>
#include <core/ContentProvider.hpp>
#include <neon/neonsession.hpp>
#include <ne_redirect.h>
#include <ne_request.h>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

namespace Davix {

//------------------------------------------------------------------------------
// Wrapper class for neon sessions - setup and tear down hooks
//------------------------------------------------------------------------------
class NeonSessionWrapper {
public:
    NeonSessionWrapper(StandaloneNeonRequest* r, NEONSessionFactory &factory, const Uri &uri, const RequestParams &p, DavixError **err)
        : _r(r) {
        _sess = factory.provideNEONSession(uri, p, err);

        if(_sess && _sess->get_ne_sess() != NULL){
            ne_hook_pre_send(_sess->get_ne_sess(), NeonSessionWrapper::runHookPreSend, (void*) this);
            ne_hook_post_headers(_sess->get_ne_sess(), NeonSessionWrapper::runHookPreReceive, (void*) this);
        }
    }

    virtual ~NeonSessionWrapper() {
        if(_sess->get_ne_sess() != NULL){
            ne_unhook_pre_send(_sess->get_ne_sess(), NeonSessionWrapper::runHookPreSend, (void*) this);
            ne_unhook_post_headers(_sess->get_ne_sess(), NeonSessionWrapper::runHookPreReceive, (void*) this);
        }
    }

    ne_session* get_ne_sess() {
        return _sess->get_ne_sess();
    }

    bool isRecycledSession() const {
        return _sess->isRecycledSession();
    }

    void do_not_reuse_this_session() {
        _sess->do_not_reuse_this_session();
    }

private:
    static void runHookPreSend(ne_request *r, void *userdata, ne_buffer *header) {
      (void) r;

      NeonSessionWrapper* wrapper = (NeonSessionWrapper*) userdata;
      BoundHooks &boundHooks = wrapper->_r->_bound_hooks;
      if(boundHooks.presendHook) {
        std::string header_line(header->data, (header->used)-1);
        boundHooks.presendHook(header_line);
      }
    }

    static void runHookPreReceive(ne_request *r, void *userdata, const ne_status *status) {
      (void) r;

      NeonSessionWrapper* wrapper = (NeonSessionWrapper*) userdata;
      BoundHooks &boundHooks = wrapper->_r->_bound_hooks;
      if(boundHooks.prereceiveHook){
        std::ostringstream header_line;
        HeaderVec headers;
        wrapper->_r->getAnswerHeaders(headers);
        header_line << "HTTP/"<< status->major_version << '.' << status->minor_version
                    << ' ' << status->code << ' ' << status->reason_phrase << '\n';

        boundHooks.prereceiveHook(header_line.str(), headers, status->code);
      }
    }

    std::unique_ptr<NEONSession> _sess;
    StandaloneNeonRequest* _r;
};

//------------------------------------------------------------------------------
// Callback for libneon to use the content provider
//------------------------------------------------------------------------------
static ssize_t content_provider_callback(void* userdata, char* buffer, size_t buflen) {
    ContentProvider *provider = static_cast<ContentProvider*>(userdata);

    if(buflen == 0) {
      if(provider->rewind()) {
        // Success
        return 0;
      }

      return 1;
    }

    return provider->pullBytes(buffer, buflen);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
StandaloneNeonRequest::StandaloneNeonRequest(NEONSessionFactory &sessionFactory, bool reuseSession,
  const BoundHooks &boundHooks, const Uri &uri, const std::string &verb, const RequestParams &params,
  const std::vector<HeaderLine> &headers, int reqFlag, ContentProvider *contentProvider,
  Chrono::TimePoint deadline)

: _session_factory(sessionFactory), _reuse_session(reuseSession), _bound_hooks(boundHooks),
  _uri(uri), _verb(verb), _params(params), _state(RequestState::kNotStarted),
  _headers(headers), _req_flag(reqFlag), _content_provider(contentProvider),
  _deadline(deadline), _neon_req(NULL), _total_read_size(0), _last_read(-1) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
StandaloneNeonRequest::~StandaloneNeonRequest() {
  markCompleted();

  if(_neon_req) {
    ne_request_destroy(_neon_req);
    _neon_req = NULL;
  }

  _session.reset();
}

//------------------------------------------------------------------------------
// Map a neon error to davix error
//------------------------------------------------------------------------------
static void neon_error_mapper(int ne_status, StatusCode::Code & code, std::string & str, const std::string &wwwAuth) {
  switch(ne_status){
    case NE_OK: {
      code = StatusCode::OK;
      str = "Status Ok";
      break;
    }
    case NE_LOOKUP: {
      code = StatusCode::NameResolutionFailure;
      str = "Domain name resolution failed";
      break;
    }
    case NE_AUTH: {
      code = StatusCode::AuthenticationError;
      str = "Authentication failed on server";
      break;
    }
    case NE_PROXYAUTH: {
      code = StatusCode::AuthenticationError;
      str = "Authentication failed on proxy";
      break;
    }
    case NE_CONNECT: {
      code = StatusCode::ConnectionProblem;
      str = "Could not connect to server";
      break;
    }
    case NE_TIMEOUT: {
      code = StatusCode::ConnectionTimeout;
      str = "Connection timed out";
      break;
    }
    case NE_FAILED: {
      code = StatusCode::SessionCreationError;
      str = "The precondition failed";
      break;
    }
    case NE_RETRY: {
      code = StatusCode::RedirectionNeeded;
      str = "Retry Request";
      break;
    }
    default: {
      code = StatusCode::UnknownError;
      str = "Unknown Error from libneon";
    }
  }

  if(!wwwAuth.empty()) {
    str += " (WWW-Authenticate: ";
    str += wwwAuth;
    str += ")";
  }
}

//------------------------------------------------------------------------------
// Start request - calling this multiple times will do nothing.
//------------------------------------------------------------------------------
Status StandaloneNeonRequest::startRequest() {
  if(_state != RequestState::kNotStarted) {
    return Status(); ;
  }

  //----------------------------------------------------------------------------
  // Have we timed out already?
  //----------------------------------------------------------------------------
  Status st = checkTimeout();
  if(!st.ok()) {
    markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Retrieve a session, create request
  //----------------------------------------------------------------------------
  DavixError* tmp_err = NULL;
  _session.reset(new NeonSessionWrapper(this, _session_factory, _uri, _params, &tmp_err));

  if(tmp_err) {
    markCompleted();
    return Status(&tmp_err);
  }

  _neon_req = ne_request_create(_session->get_ne_sess(), _verb.c_str(), _uri.getPathAndQuery().c_str());

  //----------------------------------------------------------------------------
  // Setup headers
  //----------------------------------------------------------------------------
  for(size_t i = 0; i < _headers.size(); i++) {
    ne_add_request_header(_neon_req, _headers[i].first.c_str(),  _headers[i].second.c_str());
  }

  //----------------------------------------------------------------------------
  // Setup flags
  //----------------------------------------------------------------------------
  ne_set_request_flag(_neon_req, NE_REQFLAG_EXPECT100, _params.get100ContinueSupport() &&
            (_req_flag & RequestFlag::SupportContinue100));
  ne_set_request_flag(_neon_req, NE_REQFLAG_IDEMPOTENT, _req_flag & RequestFlag::IdempotentRequest);

  if( (_req_flag & RequestFlag::SupportContinue100) == true) {
    _session->do_not_reuse_this_session();
  }

  //----------------------------------------------------------------------------
  // Setup HTTP body
  //----------------------------------------------------------------------------
  if(_content_provider) {
    _content_provider->rewind();
    ne_set_request_body_provider(_neon_req, _content_provider->getSize(),
      content_provider_callback, _content_provider);
  }

  //----------------------------------------------------------------------------
  // Initiate the network connection
  //----------------------------------------------------------------------------
  int status = ne_begin_request(_neon_req);

  if(status != NE_OK && status != NE_REDIRECT) {
    //--------------------------------------------------------------------------
    // Network trouble, don't re-use session
    //--------------------------------------------------------------------------
    Status st = createError(status);
    _session->do_not_reuse_this_session();
    markCompleted();
    return st;
  }

  //----------------------------------------------------------------------------
  // Connection OK, we're good to go
  //----------------------------------------------------------------------------
  _state = RequestState::kStarted;
  return Status();
}

//------------------------------------------------------------------------------
// Finish an already started request.
//------------------------------------------------------------------------------
Status StandaloneNeonRequest::endRequest() {
  markCompleted();
  return Status();
}

//------------------------------------------------------------------------------
// Check if timeout has passed
//------------------------------------------------------------------------------
Status StandaloneNeonRequest::checkTimeout() {
  if(_deadline.isValid() && _deadline < Chrono::Clock(Chrono::Clock::Monolitic).now()) {
    std::ostringstream ss;
    ss << "timeout of " << _params.getOperationTimeout()->tv_sec << "s";
    return Status(davix_scope_http_request(), StatusCode::OperationTimeout, ss.str());
  }

  return Status();
}

//------------------------------------------------------------------------------
// Major read function - read a block of max_size bytes (at max) into buffer.
//------------------------------------------------------------------------------
dav_ssize_t StandaloneNeonRequest::readBlock(char* buffer, dav_size_t max_size, Status& st) {

  if(!_neon_req) {
    st = Status(davix_scope_http_request(), StatusCode::AlreadyRunning, "Request has not been started yet");
    return -1;
  }

  if(max_size == 0) {
    return 0;
  }

  if(_last_read == 0) {
    return 0;
  }

  st = checkTimeout();
  if(!st.ok()) {
    return -1;
  }

  _last_read = ne_read_response_block(_neon_req, buffer, max_size);
  if(_last_read < 0) {
    st = Status(davix_scope_http_request(), StatusCode::ConnectionProblem, "Invalid read in request");
    _session->do_not_reuse_this_session();
    markCompleted();
    return -1;
  }

  DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_HTTP, "StandaloneNeonRequestNeonRequest::readBlock read {} bytes", _last_read);

  _total_read_size += _last_read;
  return _last_read;
}

//------------------------------------------------------------------------------
// Check request state
//------------------------------------------------------------------------------
RequestState StandaloneNeonRequest::getState() const {
  return _state;
}

//------------------------------------------------------------------------------
// Get a specific response header
//------------------------------------------------------------------------------
bool StandaloneNeonRequest::getAnswerHeader(const std::string &header_name, std::string &value) const {
  if(_neon_req){
    const char* answer_content = ne_get_response_header(_neon_req, header_name.c_str());
    if(answer_content) {
      value = answer_content;
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------------------
// Get all response headers
//------------------------------------------------------------------------------
size_t StandaloneNeonRequest::getAnswerHeaders( HeaderVec & vec_headers) const {
  if(_neon_req) {
    void * handle = NULL;
    const char* name = NULL, *value = NULL;
    while( (handle = ne_response_header_iterate(_neon_req, handle, &name, &value)) != NULL){
      vec_headers.push_back(std::pair<std::string, std::string>(name, value));
    }
  }

  return vec_headers.size();
}

//------------------------------------------------------------------------------
// Mark request as completed, release any resources
//------------------------------------------------------------------------------
void StandaloneNeonRequest::markCompleted() {
  if(_state == RequestState::kFinished) {
    return;
  }

  _state = RequestState::kFinished;

  if(_neon_req) {
    if(_last_read == 0) {
      ne_end_request(_neon_req);
    }
    else {
      ne_abort_request(_neon_req);
      _session->do_not_reuse_this_session();
    }
  }
}

//------------------------------------------------------------------------------
// Create davix error object based on errors in the current session,
// or request
//------------------------------------------------------------------------------
Status StandaloneNeonRequest::createError(int ne_status) {
  StatusCode::Code code;
  std::string str, wwwAuth;
  this->getAnswerHeader("WWW-Authenticate", wwwAuth);

  if(ne_status == NE_ERROR && _session) {
    const char * neon_error = ne_get_error(_session->get_ne_sess());
    str = std::string("(Neon): ").append((neon_error)?(neon_error):"");
    if (str.find("SSL handshake failed") == std::string::npos) {
      code = StatusCode::ConnectionProblem;
    }
    else {
      code = StatusCode::SSLError;
    }
  }
  else {
    neon_error_mapper(ne_status, code, str, wwwAuth);
  }

  return Status(davix_scope_http_request(), code, str);
}

//------------------------------------------------------------------------------
// Get status code - returns 0 if impossible to determine
//------------------------------------------------------------------------------
int StandaloneNeonRequest::getStatusCode() const {
  if(_neon_req) {
    return ne_get_status(_neon_req)->code;
  }

  return 0;
}

//------------------------------------------------------------------------------
// Do not re-use underlying session
//------------------------------------------------------------------------------
void StandaloneNeonRequest::doNotReuseSession() {
  if(_session) {
    _session->do_not_reuse_this_session();
  }
}

//------------------------------------------------------------------------------
// Has the underlying session been used before?
//------------------------------------------------------------------------------
bool StandaloneNeonRequest::isRecycledSession() const {
  if(_session) {
    return _session->isRecycledSession();
  }

  // We don't have a session at all, so by definition we can't be using any
  // recycled session
  return false;
}

//------------------------------------------------------------------------------
// Get session error, if available
//------------------------------------------------------------------------------
std::string StandaloneNeonRequest::getSessionError() const {
    if(!_session) {
        return std::string();
    }

    const char *neon_error = ne_get_error(_session->get_ne_sess());
    if(neon_error) {
      return std::string(neon_error);
    }

    return std::string();
}

//------------------------------------------------------------------------------
// Obtain redirected location, store into the given Uri
//------------------------------------------------------------------------------
Status StandaloneNeonRequest::obtainRedirectedLocation(Uri &out) {
  if(!_neon_req) {
    return Status(davix_scope_http_request(), StatusCode::InvalidArgument, "Request not active, impossible to obtain redirected location");
  }

  void * handle = NULL;
  const char* name = NULL, *value = NULL;
  while( (handle = ne_response_header_iterate(_neon_req, handle, &name, &value)) != NULL) {
    if(strcasecmp("location", name) == 0) {
      out = Uri(value);
      return Status();
    }
  }

  return Status(davix_scope_http_request(), StatusCode::InvalidArgument, "Could not find Location header in answer headers");
}

}
