#include <davix.hpp>
#include <davixcontext.hpp>
#include <params/davixrequestparams.hpp>
#include <auth/davixauth.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace Davix;

// instanciate and play with gates
// TEST(CredTest, basicLoad){
//     DavixError* tmp_err=NULL;
//     davix_set_log_level(DAVIX_LOG_DEBUG | DAVIX_LOG_WARNING | DAVIX_LOG_TRACE);

//     Context c;
//     X509Credential cred;

//     ASSERT_FALSE(cred.hasCert());

//     string cert_path(TEST_VALID_CERT);
//     string cert_pass(TEST_VALID_CERT_PASS);

//     cout << " cred: " << cert_path << "pass " << cert_pass << endl;

//     int ret = cred.loadFromFileP12(cert_path.c_str(), cert_pass.c_str(), &tmp_err);
//     if(tmp_err){
//         cout << "err :" << tmp_err->getErrMsg() << endl;
//     }
//     ASSERT_EQ(0, ret);
//     ASSERT_EQ(NULL, tmp_err);
//     ASSERT_TRUE(cred.hasCert());

//     X509Credential cred2(cred);
//     ASSERT_TRUE(cred2.hasCert());

//     ret = cred.loadFromFileP12("/random/stupid/path", "", &tmp_err);
//     ASSERT_EQ(-1, ret);
//     ASSERT_EQ(StatusCode::CredentialNotFound, tmp_err->getStatus());
//     DavixError::clearError(&tmp_err);
//     ASSERT_FALSE(cred.hasCert());
//     ASSERT_TRUE(cred2.hasCert());

//     ret = cred.loadFromFileP12(cert_path.c_str(), "", &tmp_err);
//     ASSERT_EQ(-1, ret);
//     ASSERT_EQ(StatusCode::LoginPasswordError, tmp_err->getStatus());
//     DavixError::clearError(&tmp_err);

// }


TEST(testAWS, awsToken){
    std::string p_key("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    std::string a_key("AKIAIOSFODNN7EXAMPLE");
    std::string req("GET\n"
                    "\n"
                    "\n"
                    "Tue, 27 Mar 2007 19:36:42 +0000\n"
                    "/johnsmith/photos/puppy.jpg");
    std::string token("AWS AKIAIOSFODNN7EXAMPLE:"
                      "bWq2s1WEIj+Ydj0vQ697zp+IXMU=");


    std::string res = getAwsAuthorizationField(req, p_key, a_key);
    ASSERT_STREQ(res.c_str(), token.c_str());

}
