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

#ifndef DAVIX_CORE_REDIRECTION_RESOLVER_HPP
#define DAVIX_CORE_REDIRECTION_RESOLVER_HPP

#include <map>
#include <mutex>
#include <utils/davix_uri.hpp>
#include <memory>
#include <libs/alibxx/containers/cache.hpp>

namespace Davix {

class RedirectionResolver {
public:
  RedirectionResolver(bool active);

  // add cached redirection
  void addRedirection(const std::string & method, const Uri & origin, std::shared_ptr<Uri> dest);

  // try to find cached redirection, resolve a full redirection chain
  std::shared_ptr<Uri> redirectionResolve(const std::string & method, const Uri & origin);

  // check if redirections are active
  bool isActive() const;

  // clean redirections
  void redirectionClean(const std::string & method, const Uri & origin);
  void redirectionClean(const Uri & origin);

private:
  bool active;

  // redirection pool
  Davix::Cache<std::pair<std::string, std::string>, Uri> redirCache;

  // resolve a single redirection chunk
  std::shared_ptr<Uri> resolveSingle(const std::string & method, const Uri & origin);
};


}

#endif