#pragma once
#ifndef TEST_FSPP_FUSE_READ_TESTUTILS_FUSEREADTEST_H_
#define TEST_FSPP_FUSE_READ_TESTUTILS_FUSEREADTEST_H_

#include "test/testutils/FuseTest.h"

class FuseReadTest: public FuseTest {
public:
  const char *FILENAME = "/myfile";

  struct ReadError {
    int error;
    size_t read_bytes;
  };

  void ReadFile(const char *filename, void *buf, size_t count, off_t offset);
  ReadError ReadFileReturnError(const char *filename, void *buf, size_t count, off_t offset);

  ::testing::Action<int(int, void*, size_t, off_t)> ReturnSuccessfulRead =
    ::testing::Invoke([](int, void *, size_t count, off_t) {
      return count;
    });

  ::testing::Action<int(int, void*, size_t, off_t)> ReturnSuccessfulReadRegardingSize(size_t filesize) {
    return ::testing::Invoke([filesize](int, void *, size_t count, off_t offset) {
      size_t ableToReadCount = std::min(count, filesize - offset);
      return ableToReadCount;
    });
  }

private:
  int OpenFile(const TempTestFS *fs, const char *filename);
};

#endif
