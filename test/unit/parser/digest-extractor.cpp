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

  v1.emplace_back("Digest", "ADLER32=03da0195");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "adler32", output));
  ASSERT_EQ(output, "03da0195");

  v1.emplace_back("Digest", "CRC32C=03da0195");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "crc32c", output));
  ASSERT_EQ(output, "03da0195");

  ASSERT_FALSE(ChecksumExtractor::extractChecksum(v1, "crc32", output));

  v1.emplace_back("Digest", "frob=y9oivLQasBUbQ4WJqkY34g==");
  ASSERT_TRUE(ChecksumExtractor::extractChecksum(v1, "frob", output));
  ASSERT_EQ(output, "cbda22bcb41ab0151b438589aa4637e2");
}
