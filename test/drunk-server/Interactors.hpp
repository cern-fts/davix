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

#ifndef DAVIX_TEST_INTERACTORS_HPP
#define DAVIX_TEST_INTERACTORS_HPP

#include "AssistedThread.hh"
#include "DrunkServer.hpp"

class LineReader;

//------------------------------------------------------------------------------
// A step-up from Interactor interface, offers a better API
//------------------------------------------------------------------------------
class BasicInteractor : public Interactor {
public:
  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~BasicInteractor();

  //----------------------------------------------------------------------------
  // Take over the specified connection - call only once. Should spawn a
  // thread in the background for reading / writing on the socket.
  //----------------------------------------------------------------------------
  virtual void handleConnection(std::unique_ptr<DrunkServer::Connection> conn);

  //----------------------------------------------------------------------------
  // Run interacting thread
  //----------------------------------------------------------------------------
  virtual void main(ThreadAssistant &assistant) = 0;

protected:
  std::unique_ptr<DrunkServer::Connection> _conn;
  std::unique_ptr<LineReader> _reader;

  AssistedThread _thread;

  //----------------------------------------------------------------------------
  // Consume single reader line
  //----------------------------------------------------------------------------
  std::string consumeLine();

};

//------------------------------------------------------------------------------
// ConnectionShutdownInteractor: Shut down the connection right after a client
// connects.
//------------------------------------------------------------------------------
class ConnectionShutdownInteractor : public BasicInteractor {
public:
  //----------------------------------------------------------------------------
  // Run interacting thread
  //----------------------------------------------------------------------------
  void main(ThreadAssistant &assistant);
};

//------------------------------------------------------------------------------
// Single-shot interactor - just a single request and response
//------------------------------------------------------------------------------
class SingleShotInteractor : public BasicInteractor {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  SingleShotInteractor(const std::string &expectedReq, const std::string &response);

  //----------------------------------------------------------------------------
  // Run interacting thread
  //----------------------------------------------------------------------------
  void main(ThreadAssistant &assistant);

protected:
  std::string _expected_request;
  std::string _response;
};

#endif
