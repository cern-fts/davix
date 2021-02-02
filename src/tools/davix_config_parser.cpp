/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN
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

#include "davix_config_parser.hpp"
#include "davix_tool_util.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()
static const std::string delimiters = " \t\n\"\'";

// find the end of a token - special handling for quotes
static size_t extract_end(const std::string &contents, const size_t start) {
  if(contents[start] == '\"' || contents[start] == '\'') {
    size_t end = contents.find(contents[start], start+1);
    if(end == std::string::npos) return end;
    end++;

    // hit into an escaped char?
    if(contents[end-2] == '\\') {
      return extract_end(contents, end-1);
    }
    return end;
  }
  else {
    size_t end = contents.find_first_of(" \t\n", start);
    if(end == std::string::npos) {
      end = contents.size()+1;
    }
    return end;
  }
}

static void escape_token(std::string &s) {
  // no need to escape
  if(s[0] != '\'' && s[0] != '\"') {
    return;
  }

  std::string replacement(1, s[0]);
  std::string target = "\\" + replacement;

  size_t index = 0;
  while( (index = s.find(target, index)) != std::string::npos) {
    s.replace(index, 2, replacement);
  }
  s = s.substr(1, s.size()-2);
}

static size_t start_token(const std::string &contents, const size_t start=0) {
  return contents.find_first_not_of(" \t\n", start);
}

static bool next_token(const std::string &contents, std::string &err, size_t start, size_t &end, size_t &next) {
  if(start == std::string::npos) {
    return false;
  }
  // tokenize by splitting on delimiters
  end = extract_end(contents, start);
  if(end == std::string::npos) {
    err = SSTR("Tokenization error (mismatched quote?) near position " << start << ":" << contents.substr(start));
    return false;
  }

  next = contents.find_first_not_of(" \t\n", end);
  return true;
}

static size_t consume_macdef(const size_t start, const std::string &contents) {
  return start_token(contents, contents.find("\n\n", start));
}

static void store_if_empty(std::string &source, const std::string &destination) {
  if(source == "")
    source = destination;
}

static bool string_starts_with(const std::string &target, const std::string &prefix) {
  if(target.size() < prefix.size()) return false;
  return std::equal(prefix.begin(), prefix.end(), target.begin());
}

static void store_option(const std::string &first, const std::string &second, Davix::Tool::OptParams &params) {
  if(first == "login") {
    store_if_empty(params.userlogpasswd.first, second);
  }
  else if(first == "password") {
    store_if_empty(params.userlogpasswd.second, second);
  }
  else if(first == "cert") {
    store_if_empty(params.cred_path, Davix::Tool::SanitiseTildedPath(second.c_str()));
  }
  else if(first == "key") {
    store_if_empty(params.priv_key, Davix::Tool::SanitiseTildedPath(second.c_str()));
  }
  else if(first == "capath") {
    params.params.addCertificateAuthorityPath(Davix::Tool::SanitiseTildedPath(second.c_str()));
  }
  else if(first == "s3accesskey") {
    store_if_empty(params.aws_auth.second, second);
  }
  else if(first == "s3secretkey") {
    store_if_empty(params.aws_auth.first, second);
  }
  else if(first == "s3region") {
    store_if_empty(params.aws_region, second);
  }
  else if(first == "s3alternate") {
    if(second == "true") params.aws_alternate = true;
  }
  else if(first == "s3token") {
    store_if_empty(params.aws_token, second);
  }
  else if(first == "azurekey") {
    store_if_empty(params.azure_key, second);
  }
  else if(first == "ostoken") {
      store_if_empty(params.os_token, second);
  }
  else if(first == "osprojectid") {
      store_if_empty(params.os_project_id, second);
  }
}

namespace Davix {

std::vector<std::string> davix_config_tokenize(const std::string &contents, std::string &err) {
  std::vector<std::string> tokens;
  size_t start = start_token(contents), end, next;
  while(next_token(contents, err, start, end, next)) {
    std::string token = contents.substr(start, end-start);
    escape_token(token);
    tokens.push_back(token);

    start = next;
  }
  return tokens;
}

// returns true if there was any match for host
bool davix_config_apply(const std::string &filename, const std::string &contents, const Uri &uri, Tool::OptParams &params) {
  std::string err;

  std::string prevtoken;
  std::string hostname;
  std::string path;

  bool active = false;
  bool in_default = false;

  size_t start = start_token(contents), end, next;
  while(next_token(contents, err, start, end, next)) {
    std::string token = contents.substr(start, end-start);

    // first token in the pair
    if(prevtoken == "") {
      // special case: ignore macro definitions
      if(token == "macdef") {
        start = consume_macdef(start, contents);
        continue;
      }
      if(token == "default") {
        if(active) return true;

        if(in_default) {
          std::cerr << "davix: Warning: Malformed config file: " << filename << ". No entries should follow after 'default'." << std::endl;
          return false;
        }
        active = true;
        in_default = true;
        path = "";
        std::cerr << "davix: using " << filename << " to load additional configuration. (match: default)" << std::endl;
      }
      else {
        prevtoken = token;
      }
    }
    // second token in the pair
    else {
      escape_token(token);

      if(prevtoken == "machine") {
        if(active) return true;
        path = "";

        if(token == uri.getHost()) {
          std::cerr << "davix: using " << filename << " to load additional configuration. (match: " << uri.getHost() << ")" << std::endl;
          hostname = token;
          active = true;
        }
      }
      else if(prevtoken == "path") {
        // only match a single explicit path!
        if(active && path != "" && string_starts_with(uri.getPath(), path)) return true;
        path = token;
      }
      else if(active && string_starts_with(uri.getPath(), path)) {
        store_option(prevtoken, token, params);
      }
      prevtoken = "";
    }
    start = next;
  }
  return active;
}

bool davix_config_apply(const std::string &filename, Tool::OptParams &params, const std::string &url) {
  Uri uri(url);
  std::string sanitized = Tool::SanitiseTildedPath(filename.c_str());
  std::ifstream t(sanitized.c_str());
  if(uri.getHost() != "" && t.good()) {
    std::string contents((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

    return davix_config_apply(filename, contents, uri, params);
  }
  return false;
}

}
