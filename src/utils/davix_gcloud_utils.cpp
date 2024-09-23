/*
 * This File is part of Davix, The IO library for HTTP based protocols
 * Copyright (C) CERN 2018
 * Authors: Georgios Bitzes <georgios.bitzes@cern.ch>
 *          Mario Lassnig <mario.lassnig@cern.ch>
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

#include <fstream>
#include <iomanip>
#include <sstream>
#include <rapidjson/document.h>
#include <libs/alibxx/crypto/hmacsha.hpp>
#include <libs/alibxx/crypto/base64.hpp>

#include <utils/davix_gcloud_utils.hpp>
#include <utils/davix_logger_internal.hpp>
#include <utils/stringutils.hpp>

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

namespace Davix{

namespace gcloud {

std::string extract_bucket(const Uri & url) {
    std::string path = url.getPath();
    std::string name = path.substr(1, path.find("/", 1));
    if(name.compare(name.size()-1,1,"/") == 0) {
        name.erase(name.size()-1, name.size());
    }
    return name;
}

std::string extract_path(const Uri & url) {
    std::string path = url.getPath();
    std::size_t sep = path.find("/", 1);
    if(sep == std::string::npos) return "";
    return path.substr(path.find("/", 1)+1, path.size());
}


class CredentialsInternal {
public:
  CredentialsInternal() {}
  std::string private_key;
  std::string client_email;
};

bool Credentials::isEmpty() const {
  return internal->private_key.empty() && internal->client_email.empty();
}

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

void Credentials::setClientEmail(const std::string &str) {
  internal->client_email = str;
}

std::string Credentials::getClientEmail() const {
  return internal->client_email;
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

  if(!document.HasMember("client_email")) {
    throw DavixException(std::string("davix::gcloud"), StatusCode::ParsingError, "Error during JSON parsing: Could not find client_email");
  }

  creds.setPrivateKey(document["private_key"].GetString());
  creds.setClientEmail(document["client_email"].GetString());

  return creds;
}

Credentials CredentialProvider::fromFile(const std::string &path) {
  std::stringstream buffer;

  try {
    std::ifstream tmp(path);
    tmp.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    buffer << tmp.rdbuf();
  }
  catch(...) {
    throw DavixException(std::string("davix::gcloud"), StatusCode::FileNotFound, SSTR("Could not read gcloud credentials at '" << path << "'"));
  }

  return fromJSONString(buffer.str());
}

Uri signURIFixedTimeout(const Credentials& creds,
			const std::string &verb,
			const Uri &url,
			const HeaderVec &headers,
			const time_t signDuration) {
  return signURI(creds, verb, url, headers, signDuration);
}

Uri signURI(const Credentials& creds,
	    const std::string &verb,
	    const Uri &url,
	    const HeaderVec &headers,
	    const time_t expirationTime) {

  // Reference: https://cloud.google.com/storage/docs/access-control/create-signed-urls-program

  // Step 1: Build canonical request
  std::ostringstream cr;
  time_t currentTime = std::time(nullptr);

  // As of 2024-08, verbs accepted by Google are DELETE, GET, HEAD, POST, PUT
  // No further check here for validity of verbs in case Google changes their mind
  cr << verb << '\n';

  // Relative path without hostname
  cr << url.getPath() << '\n';

  // Mandatory parameters
  // 1: Two possible algorithms, GOOG4-HMAC-SHA256 and GOOG4-RSA-SHA256. We can force the RSA
  cr << "X-Goog-Algorithm=GOOG4-RSA-SHA256&";
  // 2: Extract and correctly encode the credential with automatic timed scoping of the request
  cr << "X-Goog-Credential=" << Uri::queryParamEscape(creds.getClientEmail())
                             << "%2F"
                             << std::put_time(std::gmtime(&currentTime), "%Y%m%d")
                             << Uri::queryParamEscape("/auto/storage/goog4_request") << "&";
  // 3: Timestamp for good measure
  cr << "X-Goog-Date=" << std::put_time(std::gmtime(&currentTime), "%Y%m%dT%H%M%SZ") << "&";
  // 4: Duration in seconds for request to get accepted
  cr << "X-Goog-Expires=" << expirationTime << "&";
  // 5: We only have one signed header, the hostname of the loadbalancer
  cr << "X-Goog-SignedHeaders=host" << '\n';
  // 6: Put the host header here
  cr << "host:" << url.getHost() << '\n';

  // For some reason an extra blank line needs to be here
  cr << '\n';

  // Signed header, again for whatever reason
  cr << "host" << '\n';

  // Allow all content in the payload
  cr << "UNSIGNED-PAYLOAD";

  // Step 2: Hash the canonical request and convert into hex
  std::ostringstream cr_hash, cr_hash_hexify;
  cr_hash << sha256(cr.str());
  for (unsigned char c : cr_hash.str()) {
      cr_hash_hexify << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
  }

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "Canonical request: {}", StrUtil::stringReplace(cr.str(), "\n", "\\n"));

  // Step 3: Construct the string to sign
  std::ostringstream ss, ss_time;
  ss << "GOOG4-RSA-SHA256" << '\n';
  ss_time << std::put_time(std::gmtime(&currentTime), "%Y%m%dT%H%M%SZ");
  ss << ss_time.str() << '\n';
  ss << ss_time.str().substr(0, 8) + "/auto/storage/goog4_request" << '\n';
  ss << cr_hash_hexify.str();

  DAVIX_SLOG(DAVIX_LOG_DEBUG, DAVIX_LOG_S3, "String to sign: {}", StrUtil::stringReplace(ss.str(), "\n", "\\n"));

  // Step 4: Sign the string and convert into hex
  std::string binarySignature = rsasha256(creds.getPrivateKey(), ss.str());
  std::ostringstream finalHex;
  for (unsigned char c : binarySignature) {
      finalHex << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
  }

  // Step 5: Signed URL must have the same set of query parameters as the original canonical request
  //         But be careful with parameter escaping
  Uri signedUrl(url);

  // Same fixed algorithm type and credential
  signedUrl.addQueryParam("X-Goog-Algorithm", "GOOG4-RSA-SHA256");
  std::ostringstream googCred;
  googCred << creds.getClientEmail() << "/" << std::put_time(std::gmtime(&currentTime), "%Y%m%d") << "/auto/storage/goog4_request";
  signedUrl.addQueryParam("X-Goog-Credential", googCred.str());

  // Same timestamp and duration
  std::ostringstream googDate;
  googDate << std::put_time(std::gmtime(&currentTime), "%Y%m%dT%H%M%SZ");
  signedUrl.addQueryParam("X-Goog-Date", googDate.str());
  signedUrl.addQueryParam("X-Goog-Expires", std::to_string(expirationTime));

  // Signed headers again
  signedUrl.addQueryParam("X-Goog-SignedHeaders", "host");

  // And finally the actual signature
  signedUrl.addQueryParam("X-Goog-Signature", finalHex.str());

  // Step 6: Done
  return signedUrl;
}

Uri getListingURI(const Uri & original_url, const RequestParams & params) {
    Uri newUri = original_url;
    newUri.setPath("/" + extract_bucket(original_url) );

    std::string filename = extract_path(original_url);
    if(filename[filename.size()-1] != '/') {
        filename.append("/");
    }

    // special case: listing top-dir
    if(filename == "/")
        filename = "";

    newUri.addQueryParam("prefix", filename);
    newUri.addQueryParam("delimiter", "/");
    newUri.addQueryParam("max-keys", "1000000000");

    return newUri;
}

} // namespace gcloud
} // namespace Davix
