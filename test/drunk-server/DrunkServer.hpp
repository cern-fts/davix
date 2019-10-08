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

#ifndef DAVIX_TEST_DRUNK_SERVER_HPP
#define DAVIX_TEST_DRUNK_SERVER_HPP

#include "AssistedThread.hh"
#include "EventFD.hh"

#include <netinet/in.h>
#include <memory>
#include <vector>
#include <thread>
#include <condition_variable>
#include <deque>
#include <mutex>

class Interactor;

//------------------------------------------------------------------------------
// A barebones server to be used for testing davix.
//------------------------------------------------------------------------------
class DrunkServer {
public:
  //----------------------------------------------------------------------------
  // Start listening for incoming connections on the given port.
  //----------------------------------------------------------------------------
  DrunkServer(int port);

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  ~DrunkServer();

  //----------------------------------------------------------------------------
  // A Connection class, returned by DrunkServer when a client connects.
  //----------------------------------------------------------------------------
  class Connection {
  public:
    //--------------------------------------------------------------------------
    // Constructor
    //--------------------------------------------------------------------------
    Connection(int fd);

    //--------------------------------------------------------------------------
    // Destructor
    //--------------------------------------------------------------------------
    ~Connection();

    //--------------------------------------------------------------------------
    // Read into buffer
    //--------------------------------------------------------------------------
    ssize_t read(char* buf, size_t count);

    //--------------------------------------------------------------------------
    // Read into string
    //--------------------------------------------------------------------------
    ssize_t read(std::string &buf, size_t count);

    //--------------------------------------------------------------------------
    // Write
    //--------------------------------------------------------------------------
    ssize_t write(const char* buf, size_t count);

    //--------------------------------------------------------------------------
    // Write string
    //--------------------------------------------------------------------------
    ssize_t write(const std::string &buf);

  private:
    int _fd;
  };

  //----------------------------------------------------------------------------
  // Accept a connection - if no clients connect within the given timeout,
  // we give up and return NULL.
  //----------------------------------------------------------------------------
  std::unique_ptr<Connection> accept(int timeoutSeconds);

  //----------------------------------------------------------------------------
  // Auto-accept next connection with the given interactor
  //----------------------------------------------------------------------------
  void autoAcceptNext(Interactor *intr);

private:

  //----------------------------------------------------------------------------
  // Run acceptor thread
  //----------------------------------------------------------------------------
  void runAcceptorThread(ThreadAssistant &assistant);

  int _port;
  int _socketFd;

  AssistedThread _acceptor_thread;
  EventFD _shutdown_fd;

  std::deque<int> _overflowFds;
  std::deque<Interactor*> _interactors;
  std::mutex _mtx;
  std::condition_variable _cv;

};

//------------------------------------------------------------------------------
// An Interactor class that fully handles a specific client connection
//------------------------------------------------------------------------------
class Interactor {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  Interactor() : _is_ok(false) {}

  //----------------------------------------------------------------------------
  // Virtual destructor
  //----------------------------------------------------------------------------
  virtual ~Interactor() {}

  //----------------------------------------------------------------------------
  // Take over the specified connection - call only once. Should spawn a
  // thread in the background for reading / writing on the socket.
  //----------------------------------------------------------------------------
  virtual void handleConnection(std::unique_ptr<DrunkServer::Connection> conn) = 0;

  //----------------------------------------------------------------------------
  // Was the interaction ok?
  //----------------------------------------------------------------------------
  bool ok() const {
    return _is_ok;
  }

protected:
  bool _is_ok;
};

#endif
