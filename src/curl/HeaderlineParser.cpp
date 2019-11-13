/*
 * this file is part of davix, the io library for http based protocols
 * copyright (c) cern 2019
 * author: georgios bitzes <georgios.bitzes@cern.ch>
 *
 * this library is free software; you can redistribute it and/or
 * modify it under the terms of the gnu lesser general public
 * license as published by the free software foundation; either
 * version 2.1 of the license, or (at your option) any later version.
 *
 * this library is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the gnu
 * lesser general public license for more details.
 *
 * you should have received a copy of the gnu lesser general public
 * license along with this library; if not, write to the free software
 * foundation, inc., 51 franklin street, fifth floor, boston, ma  02110-1301  usa
 *
*/

#include "HeaderlineParser.hpp"

namespace Davix {

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
HeaderlineParser::HeaderlineParser(const char *buff, size_t len) {
  parse(buff, len);
}

HeaderlineParser::HeaderlineParser(const std::string &str) {
  parse(str.c_str(), str.size());
}

//------------------------------------------------------------------------------
// Main parse function
//------------------------------------------------------------------------------
void HeaderlineParser::parse(const char *buff, size_t len) {
  if(buff == NULL || len == 0) {
    return;
  }

  if(len >= 1 && buff[len-1] == '\0') {
    len -= 1;
  }

  if(len >= 2 && buff[len-2] == '\r' && buff[len-1] == '\n') {
    len -= 2;
  }

  size_t pos = 0;
  while(pos < len) {
    if(buff[pos] == ':') {
      key = std::string(buff, pos);
      pos++;
      break;
    }

    pos++;
  }

  if(pos == len) {
    // No ":", no key-value separation
    key = std::string(buff, len);
    return;
  }

  // skip leading value whitespace
  while(pos < len && buff[pos] == ' ') {
    pos++;
  }

  // consume value
  value = std::string(buff+pos, len-pos);
}


//------------------------------------------------------------------------------
// Get key
//------------------------------------------------------------------------------
std::string HeaderlineParser::getKey() const {
  return key;
}

//------------------------------------------------------------------------------
// Get value
//------------------------------------------------------------------------------
std::string HeaderlineParser::getValue() const {
  return value;
}

}
