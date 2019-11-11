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

#ifndef DAVIX_CORE_SESSION_POOL_HPP
#define DAVIX_CORE_SESSION_POOL_HPP

#include <map>
#include <mutex>

//------------------------------------------------------------------------------
// Utility class to juggle sessions based on URI and parameters.
//------------------------------------------------------------------------------
template<typename T>
class SessionPool {
public:
  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------
  SessionPool() {}

  //----------------------------------------------------------------------------
  // Destructor
  //----------------------------------------------------------------------------
  virtual ~SessionPool() {
    clear();
  }

  //----------------------------------------------------------------------------
  // Insert
  //----------------------------------------------------------------------------
  void insert(const std::string &key, T item) {
    std::lock_guard<std::mutex> lock(_mutex);
    _map.insert(std::pair<std::string, T>(key, std::move(item)));
  }

  //----------------------------------------------------------------------------
  // Clear
  //----------------------------------------------------------------------------
  void clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _map.clear();
  }

  //----------------------------------------------------------------------------
  // Retrieve: Remove from the map and copy value onto caller variable.
  // Return true if value was found, returned and erased, and false otherwise.
  //----------------------------------------------------------------------------
  bool retrieve(const std::string &key, T& item) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _map.find(key);

    if(it == _map.end()) {
      return false;
    }

    item = it->second;
    _map.erase(it);
    return true;
  }

private:
  std::multimap<std::string, T> _map;
  std::mutex _mutex;

};

#endif

