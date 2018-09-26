#include <davix.hpp>
#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <auth/davixauth.hpp>
#include <gtest/gtest.h>
#include <utils/davix_gcloud_utils.hpp>
#include <libs/alibxx/crypto/hmacsha.hpp>
#include <libs/alibxx/crypto/base64.hpp>

using namespace std;
using namespace Davix;

#define SSTR(message) static_cast<std::ostringstream&>(std::ostringstream().flush() << message).str()

// FUN FACT: This is actually my private SSH key
std::string privKey =
"-----BEGIN RSA PRIVATE KEY-----\\n"
"MIIEpgIBAAKCAQEAoiaxopVzTxog3x44cFJKmKZ4zPRceGU+CIuU54gMBvz0V3dk\\n"
"OJw3pwY6EW3/psDPfBpIQNUawOUcS+WlG/7AYzPo5D432xAO47I1DfJ/sk7vPZF7\\n"
"WBZOLwPBKShi+7BxES6blSSyE0KBISKFAE8S5r6PImQg7+6v9aKvYwjKniUAP780\\n"
"6UHcgA0f2XL56xnWfVk/qX654EqWq0JYGbXPfN/zTpj5zEza59oMKSIZryoJbeZP\\n"
"FYaryT4oTI5aoPk/6RuGJ4ga9LG0Xg/DCQ3dMyyAZAEpRqMyIvg+RkzodefVA7vb\\n"
"y6gX+ttXFRbndEGg06tX0MXscgo9j+CngHXwAwIDAQABAoIBAQCUbULxBkiUweAR\\n"
"HIKhHlK2RLwTMCNs+Y+9Frfo8DnCM84eOmV9BFJVC3gf+EaqB5hli1FUNz3UEGTE\\n"
"fKOUaq8En8GxG9rZJWvbtYanC7EkMOkBvKfhTBmpI5Cu8g8ZQLte9QDplruCsb+t\\n"
"6wE/9SHiIdC3tFwKBL0A5jgROTzvYI5HleWB2fc9emU9RaEjFl6eQBqd3bPPSFVo\\n"
"3mUctVFEG76FMAytYSb+kLpO3aiPvMqrwOoK5N/PqaWH5bh392b14BoqaPQNidq9\\n"
"XX2emrbaz7G1RswNPaOsJ6qpwWc+1zibkd3BwSdT3HdOX/7G6cUEnv5ibZ3SsY/n\\n"
"5Z/89lMxAoGBAMwmQ5o2iJgwbU2Z5B1IKRNI1TExu1R8O+xEdT7VdG3aGScoFvkT\\n"
"oEHmXKZtjYYgq5VwcOzSklROOUqto5VCSt2BCXwE7qms9+wspMVE7UQNh/LHdVBT\\n"
"LuKrhMsuF4/E6sAHsNjip4+gZkHFiJu67DXxgS0Et4+r1suVsgtft4pfAoGBAMtV\\n"
"tjvAA7pZXS/4R9V5B0UZ+mvdIRMuDHpPxnVv8Wb9VaQXxA/5tbPiClqLn6yMiKIg\\n"
"ay1YzK191EoQuQMEiaNpCfT+DH0xTihU9KcWNGSD5fYZL4nVac8LhfU+V2VWvXdc\\n"
"fgUjrLVLBNRDiInYNceLK36K5tqb/Z0sigSBZgTdAoGBAKDgInPbN6ceunPlqtK8\\n"
"c8oDoiVjOGlqgVo91FsQoPCUZy/pMrlTkv17iFWKjXU+N5jLA+JMSg3vmsxTq05K\\n"
"8G4/anb1+BF0Aed2gt4F5Ce1tgVG0xbahl8PpNTsXJrqJcfwB5fSfRL85rg77twx\\n"
"4ETeLNqKFhE87EaAj87QvfVnAoGBAL6dOjmnFpeSAL7gdI7VZ5BK+yw03zW4vG/I\\n"
"61XaPCZ9JNSwhtcnE0RviZ86NtSt5cR+uZqIHVAinmlUZexDS7hJ1tC5fAG8v/Ul\\n"
"NziPo0v7Lg/Xqm3/B7LVrZ2q9IkGmJUVuvmcaOKHUh6etJfsfPX0LDDzi9ix1T2L\\n"
"rFLu7zFtAoGBAInoW7w0uyNiFGCjoDPvHlQTGZrk2hY2lyFJQjpWqwx5qq7917gp\\n"
"GQ27cF/m5ZAMDtr21CH648NIp1rrtNt9WgF9WRMVjILas558VPQgk/oiMGWS7zH/\\n"
"bcg/HecqmrFwg7XyixqXxDCqKLlf8O7XzzGLAjowMRCBQiLwixtJAhUi\\n"
"-----END RSA PRIVATE KEY-----\\n";

static bool startswith(const std::string &str, int start, const std::string &prefix) {
  if(prefix.size() > str.size() + start) return false;

  for(size_t i = 0; i < prefix.size(); i++) {
    if(str[start++] != prefix[i]) return false;
  }

  return true;
}

static std::string replace(const std::string &source, const std::string &target, const std::string &replacement) {
  std::stringstream ss;

  for(size_t i = 0; i < source.size(); i++) {
    if(startswith(source, i, target)) {
      ss << replacement;
      i += target.size() - 1;
    }
    else {
      ss << source[i];
    }
  }

  return ss.str();
}

TEST(GcloudTest, CredParsing) {
  gcloud::CredentialProvider credProvider;
  ASSERT_THROW(credProvider.fromJSONString("aaaaa"), DavixException);
  ASSERT_THROW(credProvider.fromJSONString(SSTR("{ \"priva_key\":\"" << privKey << "\" }")), DavixException);

  std::string jsonString = SSTR("{ \"client_email\": \"aaa@bb.gserviceaccount.com\", \"private_key\":\"" << privKey << "\" }");
  gcloud::Credentials creds = credProvider.fromJSONString(jsonString);

  ASSERT_EQ(creds.getPrivateKey(), replace(privKey, "\\n", "\n"));
  ASSERT_EQ(creds.getClientEmail(), "aaa@bb.gserviceaccount.com");
}

TEST(GcloudTest, RsaSha256Signature) {
  std::string unencoded = rsasha256(replace(privKey, "\\n", "\n"), "super secure message");
  std::string encoded = Base64::base64_encode( (const unsigned char*) unencoded.c_str(), unencoded.size());
  ASSERT_EQ(encoded, "Dqyv8/2sQyTWtVGTkxgXe0zKQGEPuK/xbhLV7tExpBPE6X4gIU5vNxIYOeK2QwgpiAN/XjEzAp9u9nZxalhyeZvf7QnXs0iiTqmmXUANHPwhWKgRHi9QG1QFONnJ8ZgSjpakQqaCXPgcXskO2zhKwY7Z91cRY42kYyCF7+aCduYOcYYbVwSUxuF6zkj/HvvGGF+eVX1+eOe59Py1mETGHv1Uf0URiCsq5W3XoU9HKl5zPX8rX/GL85w0WUUpUsyT1BcVaC3CJtpLOcHxeovO6RziyYNg5vOzsidjl0nETEtjoFIpLQva+ozok2xh2+dGHTkc9ZDkjMjZK7h/KB1s8Q==");
}

TEST(GcloudTest, UrlSigning) {
  gcloud::CredentialProvider credProvider;
  gcloud::Credentials creds = credProvider.fromJSONString(SSTR("{ \"client_email\": \"aaa@bb.gserviceaccount.com\", \"private_key\":\"" << privKey << "\" }"));

  HeaderVec hv;

  Uri signedUrl = gcloud::signURIFixedTimeout(creds, "GET", Uri("https://storage.googleapis.com/random-bucket/aaaaa"), hv, 1525107409);
  ASSERT_EQ(signedUrl.getString(), "https://storage.googleapis.com/random-bucket/aaaaa?GoogleAccessId=aaa%40bb.gserviceaccount.com&Expires=1525107409&Signature=MaYqh3Ysjw8trX1H%2BO%2BNBxn4I3cSfKsQNJRrltH6apJpNq7AitI0YvEZ4fFoGcPhb3oz13kyxlsDB1v61%2FlFXJLUFHD6erpMubYMTEeAjw50NavcJtoXNUmHXUAnwj164ffD%2B8VQBK9UIOgcrdh3dGumsjwQe%2F7znk6TcXr6D9vkA3uRldFNNcZBnq%2Bv1rULNzi1XHm2wCvXy%2B5vXJ0%2FDsxtlfDfleC1NEcHS8vmWbsyUwFWrZUM%2BeiIjolmWL6NZpyEEMOBnNOgmi9BcSxDr0WGDEmdh6wvSUvbJzknZrKjgIj%2BH4AaYFOKsCV946mg0whkXc4F7lxpmzax5HHfqQ%3D%3D");
}
