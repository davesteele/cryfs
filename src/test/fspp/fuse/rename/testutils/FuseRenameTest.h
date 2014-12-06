#pragma once
#ifndef TEST_FSPP_FUSE_RENAME_TESTUTILS_FUSERENAMETEST_H_
#define TEST_FSPP_FUSE_RENAME_TESTUTILS_FUSERENAMETEST_H_

#include "test/testutils/FuseTest.h"

class FuseRenameTest: public FuseTest {
public:
  const char *FILENAME1 = "/myfile1";
  const char *FILENAME2 = "/myfile2";

  void Rename(const char *from, const char *to);
  int RenameReturnError(const char *from, const char *to);
};

#endif
