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

#include "RedirectionResolver.hpp"
#include <utils/davix_logger_internal.hpp>


using namespace Davix;

static const std::pair<std::string, std::string> makeKey(const std::string & method, const Uri & origin){
    std::string mymethod = method;
    // cache HEAD and GET on same key
    if(mymethod == "HEAD")
        mymethod = "GET";

    return std::make_pair(origin.getString(), mymethod);
}

RedirectionResolver::RedirectionResolver(bool act) : active(act), redirCache(256) {
  DAVIX_SLOG(DAVIX_LOG_TRACE, DAVIX_LOG_CORE, "Redirection Session caching {}", (active?"ENABLED":"DISABLED"));
}

// add cached redirection
void RedirectionResolver::addRedirection(const std::string & method, const Uri & origin, std::shared_ptr<Uri> dest) {
  if(!active) {
    return;
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Add cached redirection <{} {} {}>", method.c_str(), origin.getString().c_str(), dest->getString().c_str());
  redirCache.insert(makeKey(method, origin), dest);
}

// try to find cached redirection, resolve a full chain
std::shared_ptr<Uri> RedirectionResolver::redirectionResolve(const std::string & method, const Uri & origin) {
  std::shared_ptr<Uri> res = resolveSingle(method, origin);
  if(res.get() != NULL) {
    std::shared_ptr<Uri> res_rec = redirectionResolve(method, *res);
    if(res_rec.get() != NULL) {
      return res_rec;
    }
  }
  return res;
}

// resolve a single redirection chunk
std::shared_ptr<Uri> RedirectionResolver::resolveSingle(const std::string & method, const Uri & origin) {
  std::shared_ptr<Uri> res = redirCache.find(makeKey(method, origin));

  if(res.get() != NULL){
    DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Found redirection  <{} {} {}>", method.c_str(), origin.getString().c_str(), res->getString().c_str());
  }
  return res;
}

// check if redirections are active
bool RedirectionResolver::isActive() const {
  return active;
}

void RedirectionResolver::redirectionClean(const std::string & method, const Uri & origin) {
  std::shared_ptr<Uri> res = redirCache.find(makeKey(method, origin));
  if(res.get() != NULL){
      DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_HTTP, "Delete Cached redirection for <{} {} {}>", method.c_str(), origin.getString().c_str(), res->getString().c_str());
      redirCache.erase(makeKey(method, origin));
      redirectionClean(method, *res);
  }
}

void RedirectionResolver::redirectionClean(const Uri & origin) {
  std::pair<std::string, std::string> query = std::make_pair(origin.getString(), "");
  while(true) {
    const std::pair<std::string, std::string> nextkey = redirCache.upper_bound(query);
      if(nextkey.first != origin.getString()) {
        break;
      }

    redirectionClean(nextkey.second, nextkey.first);
  }
}
