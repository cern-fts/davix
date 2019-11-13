/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2019
 * Author: Georgios Bitzes <georgois.bitzes@cern.ch>
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

#ifndef DAVIX_CURL_HEADERLINE_PARSER_HPP
#define DAVIX_CURL_HEADERLINE_PARSER_HPP

#include <string>

namespace Davix {

//------------------------------------------------------------------------------
// Utility class to parse header lines, and split them into key and value
//------------------------------------------------------------------------------
class HeaderlineParser {
public:
  HeaderlineParser(const char *buff, size_t len);
  HeaderlineParser(const std::string &str);

  std::string getKey() const;
  std::string getValue() const;

private:
  void parse(const char *buff, size_t len);
  std::string key;
  std::string value;
};

}

#endif
