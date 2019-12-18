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

#include <gtest/gtest.h>
#include <backend/StandaloneNeonRequest.hpp>
#include <neon/neonsessionfactory.hpp>
#include "../drunk-server/DrunkServer.hpp"
#include "../drunk-server/LineReader.hpp"
#include "../drunk-server/Interactors.hpp"
#include "test-utils.hpp"
#include <iostream>

#define DBG(message) std::cerr << __FILE__ << ":" << __LINE__ << " -- " << #message << " = " << message << std::endl;

using namespace Davix;

class Standalone_Neon_Request : public DavixTestFixture {};
class Standalone_Curl_Request : public DavixTestFixture {};

TEST_F(Standalone_Neon_Request, BasicSanity) {
  _headers.push_back(HeaderLine("I like", "Turtles"));
  _uri = Uri("http://localhost:22222/chickens");

  SingleShotInteractor inter(
    SSTR("GET /chickens HTTP/1.1\r\n"  <<
          getDefaultUserAgent()        <<
          "Keep-Alive: \r\n"           <<
          "Connection: Keep-Alive\r\n" <<
          "TE: trailers\r\n"           <<
          "Host: localhost:22222\r\n"  <<
          "I like: Turtles\r\n"        <<
          "\r\n"),

    SSTR("HTTP/1.1 200 OK\r\n"                       <<
         "Date: Mon, 07 Oct 2019 14:02:25 GMT\r\n"   <<
         "Content-Type: ayy/lmao\r\n"                <<
         "Content-Length: 19\r\n"                    <<
         "\r\n"                                      <<
         "I like turtles too.\r\n")
  );

  _drunk_server->autoAcceptNext(&inter);

  std::unique_ptr<StandaloneRequest> request = makeStandaloneNeonReq();
  ASSERT_FALSE(request->isRecycledSession());
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);

  ASSERT_TRUE(request->startRequest().ok());
  ASSERT_EQ(request->getState(), RequestState::kStarted);
  ASSERT_EQ(request->getStatusCode(), 200);

  std::string headerLine;
  ASSERT_TRUE(request->getAnswerHeader("Content-Type", headerLine));
  ASSERT_EQ(headerLine, "ayy/lmao");

  Uri redirect;
  ASSERT_FALSE(request->obtainRedirectedLocation(redirect).ok());

  sleep(1); // yes this is a hack to be replaced

  char buffer[2048];

  Status st;
  ASSERT_EQ(request->readBlock(buffer, 2048, st), 19);
  ASSERT_TRUE(st.ok());
  ASSERT_EQ(std::string(buffer, 19), "I like turtles too.");
  ASSERT_EQ(request->readBlock(buffer, 2048, st), 0);
  ASSERT_TRUE(st.ok());

  ASSERT_EQ(request->getState(), RequestState::kStarted);
  st = request->endRequest();
  ASSERT_EQ(request->getState(), RequestState::kFinished);
  ASSERT_TRUE(st.ok());
  ASSERT_TRUE(inter.ok());
  ASSERT_EQ(request->getStatusCode(), 200);
  ASSERT_FALSE(request->isRecycledSession());
}

TEST_F(Standalone_Curl_Request, BasicSanity) {
  _headers.push_back(HeaderLine("I like", "Turtles"));
  _uri = Uri("http://localhost:22222/chickens");

  SingleShotInteractor inter(
    SSTR("GET /chickens HTTP/1.1\r\n"  <<
          "Host: localhost:22222\r\n"  <<
          "Accept: */*\r\n"            <<
          "I like: Turtles\r\n"),

    SSTR("HTTP/1.1 200 OK\r\n"                       <<
         "Date: Mon, 07 Oct 2019 14:02:25 GMT\r\n"   <<
         "Content-Type: ayy/lmao\r\n"                <<
         "Content-Length: 19\r\n"                    <<
         "\r\n"                                      <<
         "I like turtles too.\r\n")
  );

  _drunk_server->autoAcceptNext(&inter);

  std::unique_ptr<StandaloneRequest> request = makeStandaloneCurlReq();
  ASSERT_FALSE(request->isRecycledSession());
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);

  ASSERT_TRUE(request->startRequest().ok());
  ASSERT_EQ(request->getState(), RequestState::kStarted);
  ASSERT_EQ(request->getStatusCode(), 200);

  std::string headerLine;
  ASSERT_TRUE(request->getAnswerHeader("Content-Type", headerLine));
  ASSERT_EQ(headerLine, "ayy/lmao");

  Uri redirect;
  ASSERT_FALSE(request->obtainRedirectedLocation(redirect).ok());

  sleep(1); // yes this is a hack to be replaced

  char buffer[2048];

  Status st;
  ASSERT_EQ(request->readBlock(buffer, 2048, st), 19);
  ASSERT_TRUE(st.ok());
  ASSERT_EQ(std::string(buffer, 19), "I like turtles too.");
  ASSERT_EQ(request->readBlock(buffer, 2048, st), 0);
  ASSERT_TRUE(st.ok());

  ASSERT_EQ(request->getState(), RequestState::kStarted);
  st = request->endRequest();
  ASSERT_EQ(request->getState(), RequestState::kFinished);
  ASSERT_TRUE(st.ok());
  ASSERT_TRUE(inter.ok());
  ASSERT_EQ(request->getStatusCode(), 200);
  ASSERT_FALSE(request->isRecycledSession());
}

TEST_F(Standalone_Neon_Request, Redirect) {
  _uri = Uri("http://localhost:22222/test");

  SingleShotInteractor inter(
    SSTR("GET /test HTTP/1.1\r\n"      <<
          getDefaultUserAgent()        <<
          "Keep-Alive: \r\n"           <<
          "Connection: Keep-Alive\r\n" <<
          "TE: trailers\r\n"           <<
          "Host: localhost:22222\r\n"  <<
          "\r\n"),

    SSTR("HTTP/1.1 307 Temporary Redirect\r\n"                <<
         "Date: Mon, 07 Oct 2019 14:02:25 GMT\r\n"            <<
         "LoCaTiON: https://example.com/redirect-test\r\n"    <<
         "\r\n")
  );

  _drunk_server->autoAcceptNext(&inter);
  std::unique_ptr<StandaloneRequest> request = makeStandaloneNeonReq();

  ASSERT_TRUE(request->startRequest().ok());

  std::string headerLine;
  ASSERT_TRUE(request->getAnswerHeader("LoCaTiON", headerLine));
  ASSERT_EQ(headerLine, "https://example.com/redirect-test");
  ASSERT_EQ(request->getStatusCode(), 307);
  ASSERT_TRUE(request->endRequest().ok());

  Uri uri;
  ASSERT_TRUE(request->obtainRedirectedLocation(uri).ok());
  ASSERT_EQ(uri.getString(), "https://example.com/redirect-test");
}

TEST_F(Standalone_Curl_Request, Redirect) {
  _uri = Uri("http://localhost:22222/test");

  SingleShotInteractor inter(
    SSTR("GET /test HTTP/1.1\r\n"  <<
          "Host: localhost:22222\r\n"  <<
          "Accept: */*\r\n" <<
          getCurlUserAgent() <<
          "\r\n"),

    SSTR("HTTP/1.1 307 Temporary Redirect\r\n"                <<
         "Date: Mon, 07 Oct 2019 14:02:25 GMT\r\n"            <<
         "LoCaTiON: https://example.com/redirect-test\r\n"    <<
         "Content-Length: 0\r\n" <<
         "\r\n")
  );

  _drunk_server->autoAcceptNext(&inter);
  std::unique_ptr<StandaloneRequest> request = makeStandaloneCurlReq();

  ASSERT_TRUE(request->startRequest().ok());

  std::string headerLine;
  ASSERT_TRUE(request->getAnswerHeader("LoCaTiON", headerLine));
  ASSERT_EQ(headerLine, "https://example.com/redirect-test");
  ASSERT_EQ(request->getStatusCode(), 307);
  ASSERT_TRUE(request->endRequest().ok());

  Uri uri;
  ASSERT_TRUE(request->obtainRedirectedLocation(uri).ok());
  ASSERT_EQ(uri.getString(), "https://example.com/redirect-test");
}


TEST_F(Standalone_Curl_Request, NetworkError) {
  setConnectionTimeout(std::chrono::seconds(1));

  ConnectionShutdownInteractor inter;
  _drunk_server->autoAcceptNext(&inter);

  std::unique_ptr<StandaloneRequest> request = makeStandaloneCurlReq();
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);

  Status st = request->startRequest();
  ASSERT_EQ(request->getState(), RequestState::kFinished);

  ASSERT_FALSE(st.ok());
  ASSERT_EQ(st.getScope(), "Davix::HttpRequest");
  ASSERT_EQ(st.getCode(), StatusCode::ConnectionProblem);
  ASSERT_EQ(st.getErrorMessage(), "curl error (52): Server returned nothing (no headers, no data)");
  ASSERT_EQ(request->getSessionError(), "curl error (52): Server returned nothing (no headers, no data)");
  ASSERT_EQ(request->getStatusCode(), 0);

  _drunk_server.reset();
}

TEST_F(Standalone_Neon_Request, NetworkError) {
  setConnectionTimeout(std::chrono::seconds(1));

  ConnectionShutdownInteractor inter;
  _drunk_server->autoAcceptNext(&inter);

  std::unique_ptr<StandaloneRequest> request = makeStandaloneNeonReq();
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);

  Status st = request->startRequest();
  ASSERT_EQ(request->getState(), RequestState::kFinished);

  ASSERT_FALSE(st.ok());
  ASSERT_EQ(st.getScope(), "Davix::HttpRequest");
  ASSERT_EQ(st.getCode(), StatusCode::ConnectionProblem);
  ASSERT_EQ(st.getErrorMessage(), "(Neon): Could not read status line: connection was closed by server");
  ASSERT_EQ(request->getSessionError(), "Could not read status line: connection was closed by server");
  ASSERT_EQ(request->getStatusCode(), 0);

  _drunk_server.reset();
}

TEST_F(Standalone_Curl_Request, StopNoStart) {
  std::unique_ptr<StandaloneRequest> request = makeStandaloneCurlReq();
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);
  ASSERT_TRUE(request->endRequest().ok());
  ASSERT_EQ(request->getState(), RequestState::kFinished);
  ASSERT_EQ(request->getStatusCode(), 0);
}

TEST_F(Standalone_Neon_Request, StopNoStart) {
  std::unique_ptr<StandaloneRequest> request = makeStandaloneNeonReq();
  ASSERT_EQ(request->getState(), RequestState::kNotStarted);
  ASSERT_TRUE(request->endRequest().ok());
  ASSERT_EQ(request->getState(), RequestState::kFinished);
  ASSERT_EQ(request->getStatusCode(), 0);
}
