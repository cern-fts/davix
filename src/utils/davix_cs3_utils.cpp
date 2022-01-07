#include <utils/davix_cs3_utils.hpp>
#include <stdlib.h>

namespace Davix{
namespace reva {

//
// Credential 
//

// Constructor
Credentials::Credentials() {
  credMap = new CredentialMap();
}

// Destructor
Credentials::~Credentials() {
  delete credMap;
  credMap = NULL;
}

// Copy constructor
Credentials::Credentials(const Credentials& other) {
  credMap = new CredentialMap(*other.credMap);
}

// Move constructor
Credentials::Credentials(Credentials&& other) {
  credMap = other.credMap;
  other.credMap = new CredentialMap();
}

// Copy assignment operator
Credentials& Credentials::operator=(const Credentials& other) {
  credMap = new CredentialMap(*other.credMap);
  return *this;
}

// Move assignment operator
Credentials& Credentials::operator=(Credentials&& other) {
  credMap = other.credMap;
  other.credMap = new CredentialMap();
  return *this;
}

bool Credentials::isEmpty() const {
  return credMap->empty();
}

RevaToken Credentials::getToken(std::string uri) const&{
    std::string tkn = "";
    if(credMap->find(uri) != credMap->end()){
        tkn = (*credMap)[uri].token;
    } 
    return tkn;
}

void Credentials::getCredentialMap(CredentialMap & cmap) const {
     for (CredentialMap::iterator itr = credMap->begin(); itr != credMap->end(); ++itr) {
       Credential c {itr->second.token, itr->second.token_write_access};
       cmap.emplace(itr->first,c);
     }
}

void Credentials::addCredentials(std::string uri, std::string token, bool token_write_access) {
    
    if(credMap->find(uri) == credMap->end()){
        Credential cred {token, token_write_access};
        credMap->emplace(uri, cred);
    }
}


//
// Credential Provider
//

void CredentialProvider::updateCredentials(Credentials &creds, std::string uri, bool token_write_access) {
    //Assuming urls with +3rd prefix have been deprecated
    char* token = getenv("REVA_TOKEN");
    if(token == NULL) {
        throw DavixException(std::string("davix::reva"), StatusCode::EnvVarNotSet, "REVA_TOKEN variable is not set");
    }

    creds.addCredentials(uri, std::string(token), token_write_access);
}

} //reva
} //Davix
