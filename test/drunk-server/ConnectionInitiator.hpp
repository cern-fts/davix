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

#ifndef DAVIX_TEST_CONNECTION_INITIATOR_HPP
#define DAVIX_TEST_CONNECTION_INITIATOR_HPP

#include <string>

//------------------------------------------------------------------------------
// Class to initiate a synchronous TCP connection towards the specified
// host+port. After a successful connection, we do _not_ manage the lifetime
// of the file descriptor.
//------------------------------------------------------------------------------
class ConnectionInitiator {
public:
  //----------------------------------------------------------------------------
  // Connect to a host:port combination, resolve hostname manually.
  //----------------------------------------------------------------------------
  ConnectionInitiator(const std::string &hostname, int port);

  bool ok() {
    return (fd > 0) && (localerrno == 0) && (error.empty());
  }

  int getFd() {
    return fd;
  }

  int getErrno() {
    return localerrno;
  }

  std::string getError() {
    return error;
  }

private:
  int fd;
  int localerrno;
  std::string error;
};

#endif
