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
