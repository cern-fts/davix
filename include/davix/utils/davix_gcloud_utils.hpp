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

#ifndef DAVIX_GCLOUD_UTILS_HPP
#define DAVIX_GCLOUD_UTILS_HPP

#include <params/davixrequestparams.hpp>

namespace Davix {
namespace gcloud {

class CredentialsInternal;
class CredentialProvider;

///
/// @class Credentials
/// @brief Gcloud credentials
///
/// Gcloud credentials
class DAVIX_EXPORT Credentials {
public:
  Credentials();

  void setPrivateKey(const std::string &key);
  std::string getPrivateKey() const;

  // Rule of five:
  Credentials(const Credentials&);                   // Copy constructor
  Credentials(Credentials&&);                        // Move constructor
  Credentials& operator=(const Credentials&);      // Copy assignment operator
  Credentials& operator=(Credentials&&);           // Move assignment operator
  virtual ~Credentials();                            // Destructor

private:
  CredentialsInternal *internal;
};

///
/// @class CredentialProvider
/// @brief Gcloud credential provider
///
/// Gcloud credential provider
class DAVIX_EXPORT CredentialProvider {
public:
  CredentialProvider();
  Credentials fromJSONString(const std::string &str);
};

Uri signURI(const GcloudCredentialPath &credpath, const Uri &url, const time_t signDuration);

} // gcloud
} // Davix



#endif // DAVIX_GCLOUD_UTILS_HPP
