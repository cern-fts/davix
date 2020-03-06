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

#ifndef DAVIX_TEST_UTILS_HPP
#define DAVIX_TEST_UTILS_HPP

#include "../drunk-server/DrunkServer.hpp"

#include <gtest/gtest.h>
#include <backend/StandaloneNeonRequest.hpp>
#include <backend/SessionFactory.hpp>
#include <curl/StandaloneCurlRequest.hpp>


#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

inline std::string getCurlUserAgent() {
  return SSTR("User-Agent: " << Davix::RequestParams().getUserAgent() << " libcurl/7.69.0-DEV\r\n");
}

inline std::string getDefaultUserAgent() {
  return SSTR("User-Agent: " << Davix::RequestParams().getUserAgent() << " neon/0.0.29\r\n");
}

class DavixTestFixture : public ::testing::Test {
public:
  DavixTestFixture() : _drunk_server(new DrunkServer(22222)),
    _uri("http://localhost:22222/"), _verb("GET"), _flags(0) {}

  std::unique_ptr<Davix::StandaloneNeonRequest> makeStandaloneNeonReq() {
    return std::unique_ptr<Davix::StandaloneNeonRequest>(
      new Davix::StandaloneNeonRequest(_factory.getNeon(), true, _boundHooks, _uri, _verb, _params, _headers, _flags, NULL, _deadline)
    );
  }

  std::unique_ptr<Davix::StandaloneCurlRequest> makeStandaloneCurlReq() {
    return std::unique_ptr<Davix::StandaloneCurlRequest>(
      new Davix::StandaloneCurlRequest(_factory.getCurl(), true, _boundHooks, _uri, _verb, _params, _headers, _flags, NULL, _deadline)
    );
  }

  void setConnectionTimeout(std::chrono::seconds dur) {
    struct timespec tm;
    tm.tv_sec = dur.count();
    tm.tv_nsec = 0;

    _params.setConnectionTimeout(&tm);
  }

  void setDeadlineFromNow(std::chrono::milliseconds dur) {
    _deadline = Davix::Chrono::Clock(Davix::Chrono::Clock::Monolitic).now() + Davix::Chrono::Duration::milliseconds(dur.count());
  }

protected:
  std::unique_ptr<DrunkServer> _drunk_server;
  Davix::SessionFactory _factory;
  Davix::BoundHooks _boundHooks;
  Davix::Uri _uri;
  std::string _verb;
  Davix::RequestParams _params;
  std::vector<Davix::HeaderLine> _headers;
  Davix::Chrono::TimePoint _deadline;
  int _flags;


};

#endif
