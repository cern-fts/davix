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

#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

class EventFD {
public:
  EventFD() {
    fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  }

  ~EventFD() {
    close();
  }

  void close() {
    if(fd >= 0) {
      ::close(fd);
      fd = -1;
    }
  }

  void wait() {
    struct pollfd polls[1];
    polls[0].fd = fd;
    polls[0].events = POLLIN;
    polls[0].revents = 0;

    poll(polls, 1, -1);
  }

  int notify(int64_t val = 1) {
    return write(fd, &val, sizeof(val));
  }

  int64_t reset() {
    int64_t tmp;
    read(fd, &tmp, sizeof(tmp));
    return tmp;
  }

  int getFD() {
    return fd;
  }
private:
  int fd;
};

#endif
