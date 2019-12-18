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

#include "Interactors.hpp"
#include "LineReader.hpp"
#include <iostream>

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
BasicInteractor::~BasicInteractor() {}

//------------------------------------------------------------------------------
// Take over the specified connection - call only once. Should spawn a
// thread in the background for reading / writing on the socket.
//------------------------------------------------------------------------------
void BasicInteractor::handleConnection(std::unique_ptr<DrunkServer::Connection> conn) {
  _conn = std::move(conn);
  _reader.reset(new LineReader(_conn.get()));
  _thread.reset(&BasicInteractor::main, this);
}

//------------------------------------------------------------------------------
// Consume single reader line
//------------------------------------------------------------------------------
std::string BasicInteractor::consumeLine() {
  std::string out;
  _reader->consumeLine(out);
  return out;
}

//------------------------------------------------------------------------------
// Run interacting thread
//------------------------------------------------------------------------------
void ConnectionShutdownInteractor::main(ThreadAssistant &assistant) {
  _reader.reset();
  _conn.reset();
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
SingleShotInteractor::SingleShotInteractor(const std::string &expectedReq, const std::string &response)
: _expected_request(expectedReq), _response(response) {}

//------------------------------------------------------------------------------
// Split utility function - move to common header eventually
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Run interacting thread
//------------------------------------------------------------------------------
void SingleShotInteractor::main(ThreadAssistant &assistant) {
  std::vector<std::string> lines = split(_expected_request, "\r\n");

  for(size_t i = 0; i < lines.size(); i++) {
    if(i == lines.size() - 1 && lines[i].empty()) {
      break;
    }

    std::string line = consumeLine();
    std::cerr << "CONSUMED: " << line << std::endl;

    lines[i] += "\r\n";

    if(line != lines[i]) {
      std::cerr << "MISMATCH" << std::endl;
      std::cerr << "'" << line << "'" << std::endl;
      std::cerr << "'" << lines[i] << "'" << std::endl;
      return;
    }
  }

  std::cout << "WRITING RESPONSE" << std::endl;

  if(_conn->write(_response) != _response.size()) {
    std::cout << "Error when writing response" << std::endl;
    return;
  }

  std::cout << "Response written successfully" << std::endl;
  _is_ok = true;
}
