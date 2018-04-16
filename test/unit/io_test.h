#ifndef IO_TEST_H_
#define IO_TEST_H_

#include "io_util.h"
#include "param.h"
#include <gtest/gtest.h>

namespace bdm {

/// Test fixture for io tests that follow the same form
/// Usage:
///
///     TEST_F(IOTest, Type) {
///       // assign value to each data member
///       Type t;
///       t.SetDataMember1(...);
///       ...
///       Type *restored = nullptr;
///
///       BackupAndRestore(t, &restored);
///
///       // verify if all data members have been restored correctly
///       EXPECT_EQ(..., restored->GetDataMember1());
///       ...
///     }
class IOTest : public ::testing::Test {
 protected:
  static constexpr char const* kRootFile = "io-test.root";

  virtual void SetUp() {
    Param::Reset();
    remove(kRootFile);
  }

  virtual void TearDown() {
    Param::Reset();
    remove(kRootFile);
  }

  /// Writes backup to file and reads it back into restored
  template <typename T>
  void BackupAndRestore(const T& backup, T** restored) {
    // write to root file
    WritePersistentObject(kRootFile, "T", backup, "new");

    // read back
    GetPersistentObject(kRootFile, "T", *restored);
  }
};

}  // namespace bdm

#endif  // IO_TEST_H_
