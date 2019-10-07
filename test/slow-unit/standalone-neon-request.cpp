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

using namespace Davix;

class TrivialInteractor : public Interactor {
public:
  TrivialInteractor() {}

  ~TrivialInteractor() {}

  virtual void handleConnection(std::unique_ptr<DrunkServer::Connection> conn) {
    _thread.reset(&TrivialInteractor::main, this, std::move(conn));
  }

  void main(std::unique_ptr<DrunkServer::Connection> conn, ThreadAssistant &assistant) {
    char buffer[1024];
    while(true) {
      size_t sz = conn->read(buffer, 1024);
      std::cout << std::string(buffer, sz);
    }
  }

private:
  AssistedThread _thread;
};

TEST(StandaloneNeonRequest, BasicSanity) {
  NEONSessionFactory factory;
  BoundHooks boundHooks;
  Uri uri("http://localhost:22222/chickens");
  std::string verb = "GET";

  RequestParams params;

  std::vector<HeaderLine> headers;
  headers.push_back(HeaderLine("I like", "Turtles"));

  int flags = 0;
  Chrono::TimePoint invalid;


  DrunkServer ds(22222);
  TrivialInteractor inter;
  ds.autoAcceptNext(&inter);

  StandaloneNeonRequest request(factory, true, boundHooks, uri, verb, params, headers, flags, NULL, invalid);
  ASSERT_EQ(request.getState(), RequestState::kNotStarted);

  DavixError **err = NULL;
  ASSERT_FALSE(err);
  request.startRequest(err);
  ASSERT_EQ(request.getState(), RequestState::kStarted);

  // char buffer[2048];
  // request.readBlock(buffer, 2048, err);


}

