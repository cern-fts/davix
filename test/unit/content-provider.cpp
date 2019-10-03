#include <gtest/gtest.h>
#include <core/ContentProvider.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Davix;

bool makeTemporaryFile(const std::string &path, const std::string &contents) {
  FILE *out = fopen(path.c_str(), "w");

  bool ok = true;
  if(::fwrite(contents.c_str(), sizeof(char), contents.size(), out) != contents.size()) {
    ok = false;
  }

  if(fclose(out) != 0) {
    ok = false;
  }

  return ok;
}


TEST(ContentProvider, Fd) {
  ASSERT_TRUE(makeTemporaryFile("/tmp/davix-tests-tmp-file", "123456789"));

  int fd = ::open("/tmp/davix-tests-tmp-file", O_RDONLY);

  FdContentProvider provider(fd);

  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getErrc(), 0);
  ASSERT_TRUE(provider.getError().empty());
  ASSERT_EQ(provider.getSize(), 9);

  char buffer[1024];

  // Read and rewind 3 times
  for(size_t i = 0; i < 3; i++) {
    ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
    ASSERT_EQ(std::string(buffer, 3), "123");

    ASSERT_EQ(provider.pullBytes(buffer, 2), 2);
    ASSERT_EQ(std::string(buffer, 2), "45");

    ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
    ASSERT_EQ(std::string(buffer, 3), "678");

    ASSERT_EQ(provider.pullBytes(buffer, 100), 1);
    ASSERT_EQ(std::string(buffer, 1), "9");

    ASSERT_EQ(provider.pullBytes(buffer, 100), 0);
    ASSERT_EQ(provider.pullBytes(buffer, 100), 0);
    ASSERT_EQ(provider.pullBytes(buffer, 100), 0);

    ASSERT_TRUE(provider.rewind());
    ASSERT_TRUE(provider.ok());

    // Read everything in one go
    ASSERT_EQ(provider.pullBytes(buffer, 100), 9);
    ASSERT_EQ(std::string(buffer, 9), "123456789");

    ASSERT_TRUE(provider.rewind());
    ASSERT_TRUE(provider.ok());
  }

  // rewind multiple times
  for(size_t i = 0; i < 10; i++) {
    ASSERT_TRUE(provider.rewind());
    ASSERT_TRUE(provider.ok());
  }

  // read 4 bytes
  ASSERT_EQ(provider.pullBytes(buffer, 4), 4);
  ASSERT_EQ(std::string(buffer, 4), "1234");

  // .. but the fd closes..
  ASSERT_EQ(close(fd), 0);

  ASSERT_EQ(provider.pullBytes(buffer, 3), -9);
  ASSERT_EQ(provider.getErrc(), 9);
  ASSERT_EQ(provider.getError(), "Bad file descriptor");

  for(size_t i = 0; i < 10; i++) {
    ASSERT_FALSE(provider.ok());
    ASSERT_FALSE(provider.rewind());
    ASSERT_EQ(provider.pullBytes(buffer, 3), -9);
  }
}

TEST(ContentProvider, FdWithOffset) {
  ASSERT_TRUE(makeTemporaryFile("/tmp/davix-tests-tmp-file", "123456789"));

  int fd = ::open("/tmp/davix-tests-tmp-file", O_RDONLY);

  FdContentProvider provider(fd, 2);
  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 7);

  char buffer[1024];

  // Read and rewind 3 times
  for(size_t i = 0; i < 3; i++) {
    ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
    ASSERT_EQ(std::string(buffer, 3), "345");
    ASSERT_TRUE(provider.rewind());
  }

  provider = FdContentProvider(fd, 30);
  ASSERT_FALSE(provider.ok());
  ASSERT_EQ(provider.getErrc(), ERANGE);
  ASSERT_EQ(provider.getError(), "Invalid offset (30) given, fd contains only 9 bytes");
  ASSERT_FALSE(provider.rewind());

  // Test with offset near the limits of the fd
  provider = FdContentProvider(fd, 8);

  for(size_t i = 0; i < 3; i++) {
    ASSERT_TRUE(provider.ok());
    ASSERT_EQ(provider.getSize(), 1);

    ASSERT_EQ(provider.pullBytes(buffer, 3), 1);
    ASSERT_EQ(std::string(buffer, 1), "9");
    ASSERT_TRUE(provider.rewind());
  }

  provider = FdContentProvider(fd, 9);
  ASSERT_FALSE(provider.ok());
  ASSERT_EQ(provider.getErrc(), ERANGE);
  ASSERT_EQ(provider.getError(), "Invalid offset (9) given, fd contains only 9 bytes");

  ASSERT_EQ(::close(fd), 0);
}

TEST(ContentProvider, FdWithMaxlen) {
  ASSERT_TRUE(makeTemporaryFile("/tmp/davix-tests-tmp-file", "123456789"));
  int fd = ::open("/tmp/davix-tests-tmp-file", O_RDONLY);

  FdContentProvider provider(fd, 0, 5);
  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 5);

  char buffer[1024];

  // Read and rewind 3 times
  for(size_t i = 0; i < 3; i++) {
    ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
    ASSERT_EQ(std::string(buffer, 3), "123");

    ASSERT_EQ(provider.pullBytes(buffer, 2), 2);
    ASSERT_EQ(std::string(buffer, 2), "45");

    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);
    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

    ASSERT_TRUE(provider.rewind());
  }

  ASSERT_EQ(provider.pullBytes(buffer, 100), 5);
  ASSERT_EQ(std::string(buffer, 5), "12345");
  ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

  ASSERT_EQ(::close(fd), 0);
}

TEST(ContentProvider, FdWithOffsetAndMaxlen) {
  ASSERT_TRUE(makeTemporaryFile("/tmp/davix-tests-tmp-file", "123456789"));
  int fd = ::open("/tmp/davix-tests-tmp-file", O_RDONLY);

  FdContentProvider provider(fd, 2, 5);
  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 5);

  char buffer[1024];

  // Read and rewind 3 times
  for(size_t i = 0; i < 3; i++) {
    ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
    ASSERT_EQ(std::string(buffer, 3), "345");

    ASSERT_EQ(provider.pullBytes(buffer, 2), 2);
    ASSERT_EQ(std::string(buffer, 2), "67");

    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);
    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

    ASSERT_TRUE(provider.rewind());
  }

  ASSERT_EQ(provider.pullBytes(buffer, 100), 5);
  ASSERT_EQ(std::string(buffer, 5), "34567");
  ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

  ASSERT_EQ(::close(fd), 0);
}

TEST(ContentProvider, FdExcessiveLen) {
  ASSERT_TRUE(makeTemporaryFile("/tmp/davix-tests-tmp-file", "123456789"));
  int fd = ::open("/tmp/davix-tests-tmp-file", O_RDONLY);

  FdContentProvider provider(fd, 1, 10000);

  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 8);

  char buffer[1024];

  // Read and rewind 3 times
  for(size_t i = 0; i < 3; i++) {
    ASSERT_EQ(provider.pullBytes(buffer, 100), 8);
    ASSERT_EQ(std::string(buffer, 8), "23456789");

    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);
    ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

    ASSERT_TRUE(provider.rewind());
  }

  ASSERT_EQ(provider.pullBytes(buffer, 100), 8);
  ASSERT_EQ(std::string(buffer, 8), "23456789");
  ASSERT_EQ(provider.pullBytes(buffer, 1), 0);

  ASSERT_EQ(::close(fd), 0);
}

TEST(ContentProvider, Buffer) {
  std::string sourceBuffer("123456789");
  char buffer[1024];

  BufferContentProvider provider(sourceBuffer.c_str(), sourceBuffer.size());
  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 9);

  ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
  ASSERT_EQ(std::string(buffer, 3), "123");

  ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
  ASSERT_EQ(std::string(buffer, 3), "456");

  ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
  ASSERT_EQ(std::string(buffer, 3), "789");

  ASSERT_EQ(provider.pullBytes(buffer, 3), 0);
  ASSERT_EQ(provider.pullBytes(buffer, 3), 0);
  ASSERT_EQ(provider.pullBytes(buffer, 3), 0);

  ASSERT_TRUE(provider.rewind());

  ASSERT_EQ(provider.pullBytes(buffer, 200), 9);
  ASSERT_EQ(std::string(buffer, 9), "123456789");

  ASSERT_EQ(provider.pullBytes(buffer, 3), 0);
  ASSERT_TRUE(provider.rewind());

  ASSERT_EQ(provider.pullBytes(buffer, 4), 4);
  ASSERT_EQ(std::string(buffer, 4), "1234");

  ASSERT_EQ(provider.pullBytes(buffer, 100), 5);
  ASSERT_EQ(std::string(buffer, 5), "56789");

  ASSERT_EQ(provider.pullBytes(buffer, 100), 0);
}

TEST(ContentProvider, OwnedBuffer) {
  OwnedBufferContentProvider provider("test");

  char buffer[1024];
  ASSERT_TRUE(provider.ok());
  ASSERT_EQ(provider.getSize(), 4);

  ASSERT_EQ(provider.pullBytes(buffer, 3), 3);
  ASSERT_EQ(std::string(buffer, 3), "tes");
}