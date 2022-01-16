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

#include <sstream>
#include <iostream>

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
#include <stdlib.h>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

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

  if ((rv = getaddrinfo(NULL, SSTR(port).c_str(), &hints, &servinfo)) != 0) {
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

  _acceptor_thread.reset(&DrunkServer::runAcceptorThread, this);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
DrunkServer::~DrunkServer() {
  ::shutdown(_socketFd, SHUT_RDWR); // kill the socket
  _shutdown_fd.notify();
  ::close(_socketFd);
}

//------------------------------------------------------------------------------
// Accept a connection - if no clients connect within the given timeout,
// we give up and return NULL.
//------------------------------------------------------------------------------
std::unique_ptr<DrunkServer::Connection> DrunkServer::accept(int timeoutSeconds) {

  std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(timeoutSeconds);

  while(std::chrono::system_clock::now() < deadline) {

    std::unique_lock<std::mutex> lock(_mtx);
    if(_overflowFds.size() > 0) {
      int clientFd = _overflowFds.front();
      std::unique_ptr<Connection> conn = std::unique_ptr<Connection>(new Connection(clientFd));
      _overflowFds.pop_front();
      return conn;
    }

    _cv.wait_until(lock, deadline);
  }

  return {};
}

//------------------------------------------------------------------------------
// Auto-accept next connection with the given interactor
//------------------------------------------------------------------------------
void DrunkServer::autoAcceptNext(Interactor *intr) {
  std::lock_guard<std::mutex> lock(_mtx);
  _interactors.emplace_back(intr);
}

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
DrunkServer::Connection::Connection(int fd) : _fd(fd) {}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
DrunkServer::Connection::~Connection() {
  ::shutdown(_fd, SHUT_RDWR);
  ::close(_fd);
}

//------------------------------------------------------------------------------
// Read into buffer
//------------------------------------------------------------------------------
ssize_t DrunkServer::Connection::read(char* buf, size_t count) {
  return ::read(_fd, buf, count);
}

//------------------------------------------------------------------------------
// Read into string
//------------------------------------------------------------------------------
ssize_t DrunkServer::Connection::read(std::string &buf, size_t count) {
  buf.resize(count);
  ssize_t out = read( (char*) buf.c_str(), count);

  if(out > 0) {
    buf.resize(out);
  }

  return out;
}

//------------------------------------------------------------------------------
// Write
//------------------------------------------------------------------------------
ssize_t DrunkServer::Connection::write(const char* buf, size_t count) {
  return ::write(_fd, buf, count);
}

//------------------------------------------------------------------------------
// Write string
//------------------------------------------------------------------------------
ssize_t DrunkServer::Connection::write(const std::string &buf) {
  return write(buf.c_str(), buf.size());
}

//------------------------------------------------------------------------------
// Run acceptor thread
//------------------------------------------------------------------------------
void DrunkServer::runAcceptorThread(ThreadAssistant &assistant) {
  while(!assistant.terminationRequested()) {
    struct pollfd polls[2];
    polls[0].fd = _socketFd;
    polls[0].events = POLLIN;
    polls[0].revents = 0;

    polls[1].fd = _shutdown_fd.getFD();
    polls[1].events = POLLIN;
    polls[1].revents = 0;

    int rpoll = poll(polls, 2, -1);

    if(polls[1].revents != 0 || rpoll < 0) {
      return;
    }

    struct sockaddr_in remote;
    socklen_t remoteSize = sizeof(remote);

    int fd = ::accept(_socketFd, (struct sockaddr*) &remote, &remoteSize);
    if(fd < 0) {
      return;
    }

    std::unique_lock<std::mutex> lock(_mtx);

    if(!_interactors.empty()) {
      Interactor *interactor = _interactors.front();
      _interactors.pop_front();

      lock.unlock();
      interactor->handleConnection(std::unique_ptr<Connection>(new Connection(fd)));
    }
    else {
      _overflowFds.emplace_back(fd);
      _cv.notify_one();
    }

  }

}

