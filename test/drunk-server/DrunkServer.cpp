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

#include "DrunkServer.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Start listening for incoming connections on the given port.
//------------------------------------------------------------------------------
DrunkServer::DrunkServer(int port) {
  struct addrinfo hints, *servinfo, *p;
  int rv, yes = 1;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((_socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }
    if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(_socketFd, p->ai_addr, p->ai_addrlen) == -1) {
      ::close(_socketFd);
      perror("bind");
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (::listen(_socketFd, 10) == -1) {
    perror("listen");
    exit(1);
  }
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
DrunkServer::~DrunkServer() {
  ::shutdown(_socketFd, SHUT_RDWR); // kill the socket
  ::close(_socketFd);
}

//------------------------------------------------------------------------------
// Accept a connection - if no clients connect within the given timeout,
// we give up and return NULL.
//------------------------------------------------------------------------------
std::unique_ptr<DrunkServer::Connection> DrunkServer::accept(int timeoutSeconds) {

  //----------------------------------------------------------------------------
  // Wait until there's an event..
  //----------------------------------------------------------------------------
  struct pollfd polls[1];
  polls[0].fd = _socketFd;
  polls[0].events = POLLIN;
  polls[0].revents = 0;

  int rpoll = poll(polls, 1, timeoutSeconds * 1000);

  //----------------------------------------------------------------------------
  // We timed-out? Exit
  //----------------------------------------------------------------------------
  if(rpoll != 1) {
    return {};
  }

  struct sockaddr_in remote;
  socklen_t remoteSize = sizeof(remote);

  //----------------------------------------------------------------------------
  // There's an event. In any case, try to obtain an fd.
  //----------------------------------------------------------------------------
  int fd = ::accept(_socketFd, (struct sockaddr*) &remote, &remoteSize);
  if(fd < 0) {
    return {};
  }

  return std::unique_ptr<DrunkServer::Connection>(new Connection(fd));
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
DrunkServer::Connection::Connection(int fd) : _fd(fd) {}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
DrunkServer::Connection::~Connection() {}

