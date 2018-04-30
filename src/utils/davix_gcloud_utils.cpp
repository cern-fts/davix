/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2018
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

#include <utils/davix_gcloud_utils.hpp>
#include <fstream>
#include <libs/rapidjson/document.h>

namespace Davix{

namespace gcloud {

class CredentialsInternal {
public:
  CredentialsInternal() {}
  std::string private_key;
};

Credentials::Credentials() {
  internal = new CredentialsInternal();
}

// Destructor
Credentials::~Credentials() {
  delete internal;
  internal = NULL;
}

// Copy constructor
Credentials::Credentials(const Credentials& other) {
  internal = new CredentialsInternal(*other.internal);
}

// Move constructor
Credentials::Credentials(Credentials&& other) {
  internal = other.internal;
  other.internal = new CredentialsInternal();

}

// Copy assignment operator
Credentials& Credentials::operator=(const Credentials& other) {
  internal = new CredentialsInternal(*other.internal);
  return *this;
}

// Move assignment operator
Credentials& Credentials::operator=(Credentials&& other) {
  internal = other.internal;
  other.internal = new CredentialsInternal();
  return *this;
}

void Credentials::setPrivateKey(const std::string &str) {
  internal->private_key = str;
}

std::string Credentials::getPrivateKey() const {
  return internal->private_key;
}

CredentialProvider::CredentialProvider() {}

Credentials CredentialProvider::fromJSONString(const std::string &str) {
  Credentials creds;

  rapidjson::Document document;
  if(document.Parse(str.c_str()).HasParseError()) {
    throw DavixException(std::string("davix::gcloud"), StatusCode::ParsingError, "Error during JSON parsing");
  }

  if(!document.HasMember("private_key")) {
    throw DavixException(std::string("davix::gcloud"), StatusCode::ParsingError, "Error during JSON parsing: Could not find private_key");
  }

  creds.setPrivateKey(document["private_key"].GetString());

  return creds;
}

Uri signURI(const GcloudCredentialPath &credpath, const Uri &url, const time_t signDuration) {

}

} // namespace gcloud
} // namespace Davix
