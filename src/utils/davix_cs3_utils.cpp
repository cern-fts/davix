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
       cmap.insert(std::pair<std::string, Credential>(itr->first,c));
     }
}

void Credentials::addCredentials(std::string uri, std::string token, bool token_write_access){
    
    if(credMap->find(uri) == credMap->end()){
        Credential cred {token, token_write_access};
        credMap->emplace(uri, cred);
    }
}


// Helper function
// To convert char* to string safely
std::string getEnvStr(std::string var){
  char* val = getenv(var.c_str());
  return val == NULL ?   std::string() : std::string(val);
}

//
// Credential Provider
//

void CredentialProvider::updateCredentials(Credentials &creds, std::string uri, bool token_write_access){
    
    //Assuming urls with +3rd prefix have been deprecated

    std::string src_url, dst_url, token;

    src_url = getEnvStr("SRC_URL");
    dst_url = getEnvStr("DST_URL");

    // Src and Dst env var need to be set as env var for the time being
    // This will be removed in the future 
  
    if (src_url == "" || dst_url == ""){
      throw DavixException(std::string("davix::reva"), StatusCode::EnvVarNotSet, "Source or Destination variable is not set");
    }

    // If any required token is not set we get an empty string 
    // When this happens copy fails with Authentication error

    if(uri.compare(dst_url) <= 0 ){
      token = getEnvStr("REVA_DST_TOKEN");
    }
    else if (uri.compare(src_url) <= 0 ){
      token = getEnvStr("REVA_SRC_TOKEN");
    }

   creds.addCredentials(uri, token, token_write_access);
    
}

} //reva
} //Davix
