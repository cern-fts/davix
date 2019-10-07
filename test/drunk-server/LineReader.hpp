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

#ifndef DAVIX_TEST_LINE_READER_HPP
#define DAVIX_TEST_LINE_READER_HPP

#include <string>
#include <deque>
#include <sstream>
#include "DrunkServer.hpp"

//------------------------------------------------------------------------------
// Wrap a connection object, allow more comfortable reading of the contents
//------------------------------------------------------------------------------
class LineReader {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  LineReader(DrunkServer::Connection *conn);

  //----------------------------------------------------------------------------
  // Consume a single line from the link.
  // Returns 0 on "no available data", negative on error.
  //----------------------------------------------------------------------------
  int consumeLine(std::string &output);

private:
  DrunkServer::Connection *_conn;
  std::ostringstream _ss;
};

#endif
