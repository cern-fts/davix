#include <davix.hpp>
#include <utils/checksum_extractor.hpp>
#include <gtest/gtest.h>

using namespace Davix;

TEST(ChecksumExtractor, BasicSanity) {
  std::string output;

  HeaderVec v1;
  ASSERT_FALSE(ChecksumExtractor::extractChecksum(v1, "md5", output));

  v1.emplace_back("Digest", "md5=8dafb171a7455ee8977515d81c90bc12");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "md5", output));
  ASSERT_EQ(output, "8dafb171a7455ee8977515d81c90bc12");

  v1.emplace_back("Digest", "sha=y9oivLQasBUbQ4WJqkY34g==");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "md5", output));
  ASSERT_EQ(output, "8dafb171a7455ee8977515d81c90bc12");

  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "sha", output));
  ASSERT_EQ(output, "cbda22bcb41ab0151b438589aa4637e2");

  v1.emplace_back("Digest", "ADLER32=9600001");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "adler32", output));
  ASSERT_EQ(output, "09600001");

  v1.emplace_back("Digest", "CRC32C=03da0195");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "crc32c", output));
  ASSERT_EQ(output, "03da0195");

  ASSERT_FALSE(ChecksumExtractor::extractChecksum(v1, "crc32", output));

  v1.emplace_back("digest", "frob=y9oivLQasBUbQ4WJqkY34g==");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "frob", output));
  ASSERT_EQ(output, "cbda22bcb41ab0151b438589aa4637e2");
}

TEST(ChecksumExtractor, MultipleReturnedChecksums) {
  std::string output;

  HeaderVec v1;
  v1.emplace_back("Digest", "adler32=10cf712f,md5=+Tvja0I4Jp7AhrZfWO7C3A==");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "adler32", output));
  ASSERT_EQ(output, "10cf712f");

  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "md5", output));
  ASSERT_EQ(output, "f93be36b4238269ec086b65f58eec2dc");
}
