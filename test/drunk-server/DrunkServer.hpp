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

#include <netinet/in.h>
#include <memory>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

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

private:
  int _port;
  int _socketFd;

  std::vector<int> _overflowFds;
  std::mutex _mtx;
  std::condition_variable _cv;

};

#endif
