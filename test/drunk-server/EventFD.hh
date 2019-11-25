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

#ifndef DAVIX_TEST_EVENT_FD_HPP
#define DAVIX_TEST_EVENT_FD_HPP

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <fcntl.h>

class EventFD {
public:
  EventFD() {
    int status = pipe(fildes);
    if(status != 0) {
      std::cerr << "davix: CRITICAL: Could not obtain file descriptors for EventFD class, errno = " << errno << std::endl;
      std::abort();
    }

    for(size_t i = 0; i < 2; i++) {
      int flags = fcntl(fildes[i], F_GETFL, 0);
      int status = fcntl(fildes[i], F_SETFL, flags | O_NONBLOCK);
      if(status != 0) {
        std::cerr << "davix: CRITICAL: Could not set file descriptor as non-blocking" << std::endl;
        std::abort();
      }
    }
  }

  ~EventFD() {
    close();
  }

  void close() {
    ::close(fildes[0]);
    ::close(fildes[1]);
  }

  void notify() {
    char val = 1;
    int rc = write(fildes[1], &val, sizeof(val));

    if (rc != sizeof(val)) {
      std::cerr << "davix: CRITICAL: could not write to EventFD pipe, return code "
                << rc << ": " << strerror(errno) << std::endl;
    }
  }

  inline int getFD() const {
    return fildes[0];
  }

  void clear() {
    while(true) {
      char buffer[128];
      int rc = ::read(fildes[0], buffer, 64);

      if(rc <= 0) {
        break;
      }
    }
  }

private:
  int fildes[2];
};

#endif
