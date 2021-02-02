#include <davix.hpp>
#include <gtest/gtest.h>
#include <tools/davix_config_parser.hpp>

using namespace Davix;

TEST(ConfigParser, Tokenizer) {
  std::vector<std::string> tokens;
  std::string err;

  tokens = davix_config_tokenize("  token1    \t\t\t\n\n token2  \t\t\n\n  \" token3 in quotes and spaces \"   ", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 3u);
  ASSERT_EQ(tokens[0], "token1");
  ASSERT_EQ(tokens[1], "token2");
  ASSERT_EQ(tokens[2], " token3 in quotes and spaces ");

  tokens = davix_config_tokenize("\"single token in quotes\"", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 1u);
  ASSERT_EQ(tokens[0], "single token in quotes");

  // evil empty string
  tokens = davix_config_tokenize("token1 \n\t\n \"\"  token3 'token4 with single quotes @@!! '   ", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 4u);
  ASSERT_EQ(tokens[0], "token1");
  ASSERT_EQ(tokens[1], "");
  ASSERT_EQ(tokens[2], "token3");
  ASSERT_EQ(tokens[3], "token4 with single quotes @@!! ");

  // escaped quotes inside arguments
  tokens = davix_config_tokenize("\"\\\"\"  \"token2 with \\\"escaped quotes\\\" \" ", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 2u);
  ASSERT_EQ(tokens[0], "\"");
  ASSERT_EQ(tokens[1], "token2 with \"escaped quotes\" ");

  // mixing quotes
  tokens = davix_config_tokenize("'token1 with \" quotes' \"token2 with ' quotes\"  'token3 with \\' quotes' \"token4 with \\\" quotes\"", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 4u);
  ASSERT_EQ(tokens[0], "token1 with \" quotes");
  ASSERT_EQ(tokens[1], "token2 with ' quotes");
  ASSERT_EQ(tokens[2], "token3 with ' quotes");
  ASSERT_EQ(tokens[3], "token4 with \" quotes");

  // mismatched quote
  tokens = davix_config_tokenize(" 'token1 \" ", err);
  ASSERT_TRUE(err != "");
  ASSERT_EQ(tokens.size(), 0u);
  err = "";

  // mismatched quote
  tokens = davix_config_tokenize(" \"token1 ' ", err);
  ASSERT_TRUE(err != "");
  ASSERT_EQ(tokens.size(), 0u);
  err = "";

  // no extra whitespace
  tokens = davix_config_tokenize("token1 token2", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 2u);
  ASSERT_EQ(tokens[0], "token1");
  ASSERT_EQ(tokens[1], "token2");

  // what a real file might look like
  tokens = davix_config_tokenize("machine dpmhead-trunk.cern.ch\n    certpath /tmp/x509up_u1000", err);
  ASSERT_EQ(err, "");
  ASSERT_EQ(tokens.size(), 4u);
  ASSERT_EQ(tokens[0], "machine");
  ASSERT_EQ(tokens[1], "dpmhead-trunk.cern.ch");
  ASSERT_EQ(tokens[2], "certpath");
  ASSERT_EQ(tokens[3], "/tmp/x509up_u1000");
}

TEST(ConfigParser, T1) {
  std::string contents;
  Uri uri("https://somehost/somepath/path2");
  Tool::OptParams params;

  // no match
  contents =
  "machine myhost\n"
  "    login \"mylogin\"\n"
  "    password \"mypass\"\n";
  ASSERT_FALSE(davix_config_apply("null", contents, uri, params));

  // match
  contents =
  "machine somehost\n"
  "    login \"mylogin\"\n"
  "    password \"mypass\"\n";
  ASSERT_TRUE(davix_config_apply("null", contents, uri, params));
  ASSERT_EQ(params.userlogpasswd.first, "mylogin");
  ASSERT_EQ(params.userlogpasswd.second, "mypass");

  // verify that existing settings are not overwritten
  contents =
  "machine somehost\n"
  "    login \"mylogin2\"\n"
  "    password \"mypass2\"\n";
  ASSERT_TRUE(davix_config_apply("null", contents, uri, params));
  ASSERT_EQ(params.userlogpasswd.first, "mylogin");
  ASSERT_EQ(params.userlogpasswd.second, "mypass");

  // match for host, but not path
  contents =
  "machine somehost\n"
  "    path /someotherpath\n"
  "        s3accesskey key\n";
  ASSERT_TRUE(davix_config_apply("null", contents, uri, params));
  ASSERT_EQ(params.aws_auth.second, "");

  // match for both host and path + generic host settings
  contents =
  "machine somehost\n"
  "    s3secretkey commonkey\n"
  "    path /someotherpath\n"
  "        s3accesskey key\n"
  "    path /somepath\n"
  "        s3accesskey correctkey\n";
  ASSERT_TRUE(davix_config_apply("null", contents, uri, params));
  ASSERT_EQ(params.aws_auth.second, "correctkey");
  ASSERT_EQ(params.aws_auth.first, "commonkey");

  // match for both host and path + swift credentials
  contents  =
  "machine somehost\n"
  "    ostoken commontoken\n"
  "    path /someotherpath\n"
  "        osprojectid id\n"
  "    path /somepath\n"
  "        osprojectid correctid\n";
  ASSERT_TRUE(davix_config_apply("null", contents, uri, params));
  ASSERT_EQ(params.os_token, "commontoken");
  ASSERT_EQ(params.os_project_id, "correctid");
}
