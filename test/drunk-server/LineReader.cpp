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

#include "LineReader.hpp"

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
LineReader::LineReader(DrunkServer::Connection *c) : _conn(c) {}

//------------------------------------------------------------------------------
// Consume a single line from the link.
// Returns 0 on "no available data", negative on error.
//------------------------------------------------------------------------------
int LineReader::consumeLine(std::string &output) {
  char buff[10];

  while(true) {
    buff[0] = '\0';
    buff[1] = '\0';

    int retval = _conn->read(buff, 1);
    if(retval != 1) {
      return retval;
    }

    _ss << buff[0];

    if(buff[0] == '\n') {
      output = _ss.str();
      _ss.str(std::string());
      return 1;
    }
  }
}
