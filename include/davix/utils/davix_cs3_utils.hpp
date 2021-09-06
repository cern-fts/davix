#ifndef DAVIX_CS3_UTILS_HPP
#define DAVIX_CS3_UTILS_HPP

#include <params/davixrequestparams.hpp>
#include <map>

namespace Davix {
namespace reva {

struct Credential {
    RevaToken token;
    bool token_write_access;
};

typedef std::map<std::string, Credential> CredentialMap;

//creates a deep copy of CredentialMap

///
/// @class Credentials
/// @brief Reva credentials
///
/// Reva credentials
class DAVIX_EXPORT Credentials {
public:
  Credentials();

  bool isEmpty() const;
  RevaToken getToken(std::string uri) const&;
  void getCredentialMap(CredentialMap &cmap) const;
  void addCredentials(std::string uri, std::string token, bool token_write_access);

  Credentials(const Credentials&);                   // Copy constructor
  Credentials(Credentials&&);                        // Move constructor
  Credentials& operator=(const Credentials&);        // Copy assignment operator
  Credentials& operator=(Credentials&&);             // Move assignment operator
  virtual ~Credentials();                            // Destructor

private:
    CredentialMap *credMap;
};


///
/// @class CredentialProvider
/// @brief Reva credential provider
///
/// Reva credential provider
class DAVIX_EXPORT CredentialProvider{
public:
    CredentialProvider(){};
    void updateCredentials(Credentials &creds, std::string uri, bool token_write_access);
};


}//Reva
}//Davix

#endif //DAVIX_CS3_UTILS_HPP
